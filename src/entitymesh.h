/*
#ifndef ENTITYMESH_H
#define ENTITYMESH_H

#include "includes.h"
#include "utils.h"
#include "entity.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"
#include "input.h"

class EntityMesh : public Entity
{
public:
	//Attributes of this class 
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector4 color;

	EntityMesh(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, Matrix44 model);

	//methods overwritten 
	void render();
	void update(float dt);
};
#endif
*/