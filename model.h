#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"

typedef struct {
	Array textures_loaded;
	Array meshes;
	string directory;
	string name;
	bool gammaCorrection;
} Model;

void freeModel(Model *model);
void drawModel(Model *model, shader shader);
void loadModel(Model *model);

#endif