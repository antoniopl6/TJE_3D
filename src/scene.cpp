#include "scene.h"

<<<<<<< Updated upstream
void Scene::addEntityMesh(const char* meshName, const char* texName, const char* shaderName, Vector4 color, Matrix44 model) {
	Mesh* mesh = Mesh::Get(meshName);
	Texture* texture = Texture::Get(texName);
	Shader* shader = Shader::Get(texName);
	EntityMesh* emesh = new EntityMesh(mesh, texture, shader, color, model);
	entities.push_back(emesh);
};
void Scene::eraseEntity(int i)
=======

Scene::Scene()
{
	instance = this;

	//Shadow Atlas
	fbo = NULL;
	shadow_atlas = NULL;

	//General features
	ambient_light = Vector3(1.f,1.f,1.f);
	main_camera = Game::instance->camera;

	//Scene triggers: We set them true just for the first iteration
	entity_trigger = true;
	shadow_visibility_trigger = true;

}

void Scene::clear()
{

}

void Scene::load() 
>>>>>>> Stashed changes
{
	entities.erase(entities.begin() + i);
};

