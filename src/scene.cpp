#include "scene.h"

void Scene::addEntityMesh(const char* meshName, const char* texName, const char* shaderName, Vector4 color, Matrix44 model) {
	Mesh* mesh = Mesh::Get(meshName);
	Texture* texture = Texture::Get(texName);
	Shader* shader = Shader::Get(texName);
	EntityMesh* emesh = new EntityMesh(mesh, texture, shader, color, model);
	entities.push_back(emesh);
};
void Scene::eraseEntity(int i)
{
	entities.erase(entities.begin() + i);
};

