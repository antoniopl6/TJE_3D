#include "scene.h"

Scene* Scene::instance = NULL;

Scene::Scene()
{
	instance = this;

	//Shadow Atlas
	fbo = NULL;
	shadow_atlas = NULL;

	//General features
	ambient_light = Vector3(1.f, 1.f, 1.f);
	main_camera = Game::instance->camera;

	//Scene triggers: We set them true just for the first iteration
	entity_trigger = true;
	shadow_visibility_trigger = true;

}

void Scene::clear()
{

}

void Scene::load()
{


}


