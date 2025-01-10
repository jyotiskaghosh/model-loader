#ifndef MESH_H
#define MESH_H

#include "include/include/cglm/mat4.h"

#include "dynamic_array.h"

#define MAX_BONE_INFLUENCE 4

typedef char *string;
typedef unsigned int shader;

// Mesh
// .................
typedef struct {
    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 Bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
} Vertex;

typedef struct {
    unsigned int id;
    string type;
    string path;
} Texture;

typedef struct {
    Array vertices;
    Array indices;
    Array textures;
    unsigned int VAO, VBO, EBO;
} Mesh;

Mesh initMesh(Array vertices, Array indices, Array textures);
void freeMesh(Mesh *mesh);
void drawMesh(Mesh *mesh, shader shader);

#endif