#include <glad/glad.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include "stb_image.h"

#include "model.h"

#define DIRECTORY_SIZE 200

void processNode(Model *model, struct aiNode *node, const struct aiScene *scene);
Mesh processMesh(Model *model, struct aiMesh *mesh, const struct aiScene *scene);
Array loadMaterialTextures(Model *model, struct aiMaterial *mat, enum aiTextureType type, string typename);
unsigned int textureFromFile(const string path, const string directory, bool gamma);

void freeModel(Model *model)
{	
	freeArray(&model->textures_loaded);

	// free meshes
	for (int i = 0; i < model->meshes.length; i++)
		freeMesh((Mesh *) getElement(model->meshes, i));

	freeArray(&model->meshes);
}

void drawModel(Model *model, shader shader)
{
	for (unsigned int i = 0; i < model->meshes.length; i++)
		drawMesh((Mesh *) getElement(model->meshes, i), shader);
}

void loadModel(Model *model)
{
	char filename[DIRECTORY_SIZE];
	sprintf(filename, "%s/%s", model->directory, model->name);

	// import model
	const struct aiScene *scene = aiImportFile(filename, aiProcess_Triangulate| aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
	// check for errors	
	if(scene->mNumMeshes <= 0) {
		printf("Failed to load model.");
		aiReleaseImport(scene);
		return;
	}
	
	// process ASSIMP's root node recursively
	processNode(model, scene->mRootNode, scene);
}

void processNode(Model *model, struct aiNode *node, const struct aiScene *scene)
{
	// process each mesh loacted at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		Mesh mesh = processMesh(model, scene->mMeshes[node->mMeshes[i]], scene);

		append(&model->meshes, (void *)&mesh);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		processNode(model, node->mChildren[i], scene);
}

Mesh processMesh(Model *model, struct aiMesh *mesh, const struct aiScene *scene)
{
	// data to fill
	Array vertices = initArray(mesh->mNumVertices, sizeof(Vertex)); 
	Array indices = initArray(mesh->mNumFaces, sizeof(unsigned int));
	Array textures = initArray(2, sizeof(Texture));

	// walk through each of the meesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		// positions
		vertex.Position[0] = mesh->mVertices[i].x;
		vertex.Position[1] = mesh->mVertices[i].y;
		vertex.Position[2] = mesh->mVertices[i].z;

		if (mesh->mNormals) {
			vertex.Normal[0] = mesh->mNormals[i].x;
			vertex.Normal[1] = mesh->mNormals[i].y;
			vertex.Normal[2] = mesh->mNormals[i].z;
		}

		if (mesh->mTextureCoords) {
			vertex.TexCoords[0] = mesh->mTextureCoords[0][i].x;
			vertex.TexCoords[1] = mesh->mTextureCoords[0][i].y;

			// tangent
			vertex.Tangent[0] = mesh->mTangents[i].x;
			vertex.Tangent[1] = mesh->mTangents[i].y;
			vertex.Tangent[2] = mesh->mTangents[i].z;

			// bitangent
			vertex.Bitangent[0] = mesh->mTangents[i].x;
			vertex.Bitangent[1] = mesh->mTangents[i].y;
			vertex.Bitangent[2] = mesh->mTangents[i].z;
		} else {
			vertex.TexCoords[0] = 0.0f;
			vertex.TexCoords[1] = 0.0f;
		}

		append(&vertices, (void *)&vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		struct aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			append(&indices, (void *)&face.mIndices[j]);
	}
    // process materials
	struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	Array diffuseMaps = loadMaterialTextures(model, material, aiTextureType_DIFFUSE, "texture_diffuse");
	for (int i = 0; i < diffuseMaps.length; i++) {
		append(&textures, getElement(diffuseMaps, i));
	}
	// 2. specular maps
	Array specularMaps = loadMaterialTextures(model, material, aiTextureType_SPECULAR, "texture_specular");
	for (int i = 0; i < specularMaps.length; i++) {
		append(&textures, getElement(specularMaps, i));
	}
	// 3. normal maps
	Array normalMaps = loadMaterialTextures(model, material, aiTextureType_HEIGHT, "texture_normal");
	for (int i = 0; i < normalMaps.length; i++) {
		append(&textures, getElement(normalMaps, i));
	}
	// 4. height maps
	Array heightMaps = loadMaterialTextures(model, material, aiTextureType_AMBIENT, "texture_height");
	for (int i = 0; i < heightMaps.length; i++) {
		append(&textures, getElement(heightMaps, i));
	}

	return initMesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
Array loadMaterialTextures(Model *model, struct aiMaterial *mat, enum aiTextureType type, string typeName)
{
	Array textures = initArray(aiGetMaterialTextureCount(mat, type), sizeof(Texture));

	for (unsigned int i = 0; i < aiGetMaterialTextureCount(mat, type); i++) {
		struct aiString str;

		aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);

		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < model->textures_loaded.length; j++)
			if (strcmp(((Texture *) getElement(model->textures_loaded, j))->path, str.data) == 0) {
				append(&textures, getElement(model->textures_loaded, j));
				skip = true;
				break;
			}

		if (!skip) {
			Texture texture;
			texture.id = textureFromFile(str.data, model->directory, model->gammaCorrection);
			texture.type = typeName;
			texture.path = str.data;
			append(&textures, (void *)&texture);
			append(&model->textures_loaded, (void *)&texture);
		}
	}

	return textures;
}

unsigned int textureFromFile(const string path, const string directory, bool gamma)
{	
	char filename[DIRECTORY_SIZE];
	sprintf(filename, "%s/%s", directory, path);

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename, &width, &height, &nrComponents, 0);

	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
		
	} else {
		printf("Texture failed to load at path: %s\n", filename);
		stbi_image_free(data);
	}

	return textureID;
}
