#include "scene.h"
#include "game.h"
#include <fstream> 

Scene* Scene::instance = NULL;

Scene::Scene()
{
	//Singleton
	instance = this;

	//General features
	scene_path = "";
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
	//Vectors sizes
	int objects_size = objects.size();
	int lights_size = lights.size();
	int sounds_size = sounds.size();
	int max_size = max(max(objects_size, lights_size), sounds_size);

	//Delete single entities
	delete(main_character);
	delete(monster);
	main_character = NULL;
	monster = NULL;

	//Delete entity vectors
	for (int i = 0; i < max_size; ++i)
	{
		if (i < objects_size)
		{
			ObjectEntity* ent = objects[i];
			delete ent;
		}
		if (i < lights_size)
		{
			LightEntity* ent = lights[i];
			delete ent;
		}
		if (i < sounds_size)
		{
			SoundEntity* ent = sounds[i];
			delete ent;
		}
	}

	//Resize vectors
	objects.resize(0);
	lights.resize(0);
	sounds.resize(0);
}

void Scene::addEntity(Entity* entity)
{
	switch (entity->entity_type)
	{
	case(EntityType::MAIN):
		main_character = (MainCharacterEntity*) entity;
		break;
	case(EntityType::MONSTER):
		monster = (MonsterEntity*)entity;
		break;
	case(EntityType::OBJECT):
		objects.push_back((ObjectEntity*)entity);
		break;
	case(EntityType::LIGHT):
		lights.push_back((LightEntity*)entity);
		break;
	}
}

void Scene::removeEntity(Entity* entity)
{
	//Only for entity vectors
	switch (entity->entity_type)
	{
	case(EntityType::OBJECT):
		for (auto it = objects.begin(); it != objects.end(); ++it) {
			if (*it == entity) objects.erase(it);
		}
		break;
	case(EntityType::LIGHT):
		for (auto it = lights.begin(); it != lights.end(); ++it) {
			if (*it == entity) lights.erase(it);
		}
		break;
	}
}

bool Scene::load(const char* scene_filepath)
{
	//JSON content var
	std::string content;

	//Read JSON content
	if (!readFile(scene_filepath, content))
	{
		cout << "ERROR: The Scene JSON has not been found at: " << scene_filepath << endl;
		return false;
	}

	//Parse JSON content
	cJSON* scene_json = cJSON_Parse(content.c_str());
	if (!scene_json)
	{
		cout << "ERROR: Scene JSON has errors" << endl;
		return false;
	}

	//Read scene properties
	ambient_light = readJSONVector3(scene_json, "ambient_light", ambient_light);

	//Read main camera properties
	Vector3 eye = readJSONVector3(scene_json, "camera_position", main_camera->eye);
	Vector3 center = readJSONVector3(scene_json, "camera_target", main_camera->center);
	float fov = readJSONNumber(scene_json, "camera_fov", main_camera->fov);
	float camera_near = readJSONNumber(scene_json, "camera_near", main_camera->near_plane);
	float camera_far = readJSONNumber(scene_json, "camera_far", main_camera->far_plane);

	//Set the parameters of the main camera
	Game* game = Game::instance;
	main_camera->lookAt(eye, center, Vector3(0.f, 1.f, 0.f));
	main_camera->setPerspective(fov, game->window_width / (float)game->window_height, camera_near, camera_far);

	//Main character JSON
	cJSON* main_json = cJSON_GetObjectItemCaseSensitive(scene_json, "main_character");
	if (main_json)
	{
		MainCharacterEntity* main_character = new MainCharacterEntity();
		main_character->load(main_json);
		main_character->camera = main_camera;
		this->main_character = main_character;
	}
	else
	{
		cout << "Main character object hasn't been found in the JSON" << endl;
		return false;
	}

	//Monster JSON
	cJSON* monster_json = cJSON_GetObjectItemCaseSensitive(scene_json, "monster");
	if (monster_json)
	{
		MonsterEntity* monster = new MonsterEntity();
		monster->load(monster_json);
		this->monster = monster;
	}
	else
	{
		cout << "Monster object hasn't been found in the JSON" << endl;
		return false;
	}

	//Objects JSON
	cJSON* objects_json = cJSON_GetObjectItemCaseSensitive(scene_json, "objects");
	if (!objects_json)
	{
		cout << "Object array hasn't been found in the JSON" << endl;
		return false;
	}

	cJSON* object_json;
	cJSON_ArrayForEach(object_json, objects_json)
	{
		ObjectEntity* object = new ObjectEntity();
		object->load(object_json);
		objects.push_back(object);
	}

	//Lights JSON
	cJSON* lights_json = cJSON_GetObjectItemCaseSensitive(scene_json, "lights");
	if (!lights_json)
	{
		cout << "Light array hasn't been found in the JSON" << endl;
		return false;
	}

	cJSON* light_json;
	cJSON_ArrayForEach(light_json, lights_json)
	{
		LightEntity* light = new LightEntity();
		light->load(light_json);
		lights.push_back(light);
	}

	//Sounds JSON
	cJSON* sounds_json = cJSON_GetObjectItemCaseSensitive(scene_json, "sounds");
	if (!sounds_json)
	{
		cout << "Sound array hasn't been found in the JSON" << endl;
		return false;
	}

	cJSON* sound_json;
	cJSON_ArrayForEach(sound_json, sounds_json)
	{
		SoundEntity* sound = new SoundEntity();
		sound->load(sound_json);
		sounds.push_back(sound);
	}

	//free memory
	cJSON_Delete(scene_json);

	return true;
}

bool Scene::save()
{
	std::string content;

	if (!readFile(scene_path, content))
	{
		cout << "ERROR: The Scene JSON has not been found at: " << scene_path << endl;
		return false;
	}

	//Create JSON
	cJSON* scene_json = cJSON_CreateObject();

	//Add scene properties
	writeJSONVector3(scene_json, "ambient_light", ambient_light);
	
	//Add main camera properties
	writeJSONVector3(scene_json, "camera_position", main_camera->eye);
	writeJSONVector3(scene_json, "camera_target", main_camera->center);
	writeJSONNumber(scene_json, "camera_fov", main_camera->fov);
	writeJSONNumber(scene_json, "camera_near", main_camera->near_plane);
	writeJSONNumber(scene_json, "camera_far", main_camera->far_plane);

	//Main Character JSON
	cJSON* main_json = cJSON_AddObjectToObject(scene_json, "main_character");
	main_character->save(main_json);
	main_character->updateBoundingBox();

	//Monster JSON
	cJSON* monster_json = cJSON_AddObjectToObject(scene_json, "monster");
	monster->save(monster_json);
	monster->updateBoundingBox();
	
	//Objects JSON
	cJSON* objects_json = cJSON_AddArrayToObject(scene_json, "objects");
	for (int i = 0; i < objects.size(); i++)
	{
		cJSON* object_json = cJSON_CreateObject();
		objects[i]->save(object_json);
		objects[i]->updateBoundingBox();
		cJSON_AddItemToArray(objects_json, object_json);
	}

	//Lights JSON
	cJSON* lights_json = cJSON_AddArrayToObject(scene_json, "lights");
	for (int i = 0; i < lights.size(); i++)
	{
		cJSON* light_json = cJSON_CreateObject();
		lights[i]->save(objects_json);
		cJSON_AddItemToArray(lights_json, light_json);
	}

	//Sounds JSON
	cJSON* sounds_json = cJSON_AddArrayToObject(scene_json, "sounds");
	for (int i = 0; i < sounds.size(); i++)
	{
		cJSON* sound_json = cJSON_CreateObject();
		sounds[i]->save(sound_json);
		cJSON_AddItemToArray(sounds_json, sound_json);
	}

	//JSON file
	ofstream json_file("data/scene.json", ofstream::binary);

	//Delete the old JSON
	json_file.clear();

	//Save the new JSON
	int json_size = 0;
	char* json_content = cJSON_Print(scene_json, &json_size);
	json_file.write(json_content, json_size);

	//Notify the success
	cout << endl << "Scene successfully saved" << endl;

	//Free memory
	cJSON_Delete(scene_json);

	return true;
}

