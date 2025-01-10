
#include <stdio.h>
#include <string.h>

#include <glad/glad.h>

#include "mesh.h"

void setupMesh(Mesh *mesh);

Mesh initMesh(Array vertices, Array indices, Array textures)
{
    Mesh mesh = {vertices, indices, textures};
    
    setupMesh(&mesh);

    return mesh;
}

void freeMesh(Mesh *mesh)
{
    freeArray(&mesh->vertices);
    freeArray(&mesh->indices);
    freeArray(&mesh->textures);
}

// render the mesh
void drawMesh(Mesh *mesh, shader shader)
{    
    // bind appropriate textures
    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr   = 1;
    unsigned int heightNr   = 1;
    
    for(unsigned int i = 0; i < mesh->textures.length; i++) {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        char number[3];

        Texture *texture = (Texture *) getElement(mesh->textures, i);

        char name[20];
        strcpy(name,  texture->type);
        
        if(strcmp(name, "texture_diffuse"))
            sprintf(number, "%d", diffuseNr++);
        else if(strcmp(name, "texture_specular"))
            sprintf(number, "%d", specularNr++); // transfer unsigned int to string
        else if(strcmp(name, "texture_normal"))
            sprintf(number, "%d", normalNr++); // transfer unsigned int to string
        else if(strcmp(name, "texture_height"))
            sprintf(number, "%d", heightNr++); // transfer unsigned int to string

        // now set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(shader, strcat(name, number)), i);

        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, ((Texture *) getElement(mesh->textures, i))->id);
    }
    
    // draw mesh
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->indices.length, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void setupMesh(Mesh *mesh)
{    
    // create buffers/arrays
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.length * sizeof(Vertex), mesh->vertices.array, GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.length * sizeof(unsigned int*), mesh->indices.array, GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}