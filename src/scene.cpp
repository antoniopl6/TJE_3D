#include "scene.h"


Scene::Scene()
{
	//Singleton
	instance = this;

	//General features
	ambient_light = Vector3(1.f,1.f,1.f);
	main_camera = Game::instance->camera;
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	//Shadow Atlas
	fbo = NULL;
	shadow_atlas = NULL;

	//Scene triggers: We set them true just for the first iteration
	camera_trigger = true;

}

void Scene::clear()
{
	//TODO
}

void Scene::load() 
{
	//Meshes
	Mesh* island_mesh = Mesh::Get("data/island.ASE");
	Mesh* foxy_mesh = Mesh::Get("data/GrimmFoxy.obj");

	//Textures
	Texture* island_texture = Texture::Get("data/island_color.tga");
	Texture* foxy_texture = Texture::Get("data/GrimmFoxy_Shell.tga");

	//Main character
	main_character = new MainCharacterEntity("Adventurer", Matrix44(), this->main_camera, NULL,NULL);

	//Monster
	//monster = new MonsterEntity();

	//Objects
	for(int i = 0; i < 3; ++i) //Ejemplo
	{
		//Foxy matrix
		Matrix44 foxy_matrix = Matrix44();
		foxy_matrix.translate(100 * i, 400, 100 * i);
		foxy_matrix.rotate(90 * i * DEG2RAD, Vector3(0.f, 1.f, 0.f));

		ObjectEntity* object = new ObjectEntity("foxy " + i,foxy_matrix,foxy_mesh,foxy_texture);
		addEntity(object);
	}
	ObjectEntity* object = new ObjectEntity("island", Matrix44(), island_mesh, island_texture);
	addEntity(object);

	//Lights
	for (int i = 0; i < 3; ++i) //Example
	{
		//Light matrix
		Matrix44 light_matrix = Matrix44();
		light_matrix.translate(100 * i, 400, 100 * i);
		light_matrix.setFrontAndOrthonormalize(Vector3(1.f, 1.f, 1.f) * i);

		LightEntity* light = new LightEntity("light " + i, Vector3(0.12, 0.34, 0.05) * (i + 1), 5 * (i + 1), 100, i, 30, 10, 1000, false, 0.001);
		addEntity(light);
	}

}

void Scene::addEntity(Entity* entity)
{
	switch (entity->entity_type)
	{
	case(EntityType::OBJECT):
		objects.push_back((ObjectEntity*)entity);
	case(EntityType::LIGHT):
		lights.push_back((LightEntity*)entity);
	}
}

void Scene::AllocateMemory()
{
	//Reserve memory
	objects.reserve(num_objects);
	lights.reserve(num_lights);
}

