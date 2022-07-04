#include "scene.h"
#include "game.h"
#include <fstream> 

Scene* Scene::instance = NULL;

Scene::Scene()
{
	//Singleton
	instance = this;

	//General features
	filename = "";
	ambient_light = Vector3(1.f, 1.f, 1.f);
	main_camera = Game::instance->main_camera;
	shader = Shader::Get("data/shaders/pixel.vs", "data/shaders/single.fs"); //Select shader to render the render calls

	//Shadow Atlas
	fbo = NULL;
	shadow_atlas = NULL;

	//Counters
	num_objects = 0;
	num_lights = 0;
	num_sounds = 0;
	num_shadows = 0;

	//Scene properties
	show_atlas = false;
	atlas_scope = 0;

	//Scene triggers: We set them true just for the first iteration


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
	case(Entity::EntityType::MAIN):
		main_character = (MainCharacterEntity*)entity;
		break;
	case(Entity::EntityType::MONSTER):
		monster = (MonsterEntity*)entity;
		break;
	case(Entity::EntityType::OBJECT):
		objects.push_back((ObjectEntity*)entity);
		num_objects++;
		break;
	case(Entity::EntityType::LIGHT):
		lights.push_back((LightEntity*)entity);
		num_lights++;
		break;
	case(Entity::EntityType::SOUND):
		sounds.push_back((SoundEntity*)entity);
		num_sounds++;
		break;
	}
}

void Scene::removeEntity(Entity* entity)
{
	if (entity == NULL)
		return;

	//Only for entity vectors
	switch (entity->entity_type)
	{
	case(Entity::EntityType::MAIN):
		if (main_character == entity) main_character = NULL;
		break;
	case(Entity::EntityType::MONSTER):
		if (monster == entity) monster = NULL;
		break;
	case(Entity::EntityType::OBJECT):
		{
			//Downcast
			ObjectEntity* object = (ObjectEntity*)entity;

			//Children
			for (auto it = object->children.begin(); it != object->children.end(); ++it) {
				auto result = find(objects.begin(), objects.end(), *it);
				if (result != objects.end())
				{
					objects.erase(result);
					num_objects--;
				}
				delete *it;
			}

			//Just in case
			object->children.clear();

			//Parent
			auto result = find(objects.begin(), objects.end(), object);
			if (result != objects.end())
			{
				objects.erase(result);
				num_objects--;
			}
		}
		break;
	case(Entity::EntityType::LIGHT):
		{
			//Downcast
			LightEntity* light = (LightEntity*)entity;

			//Light removal
			auto result = find(lights.begin(), lights.end(), light);
			if (result != lights.end())
			{
				lights.erase(result);
				num_lights--;
			}
		}
		break;
	case(Entity::EntityType::SOUND):
		{
			//Downcast
			SoundEntity* sound = (SoundEntity*)entity;

			//Sound removal
			auto result = find(sounds.begin(), sounds.end(), sound);
			if (result != sounds.end())
			{
				sounds.erase(result);
				num_sounds--;
			}
		}
		break;
	}

	delete entity;
}

void Scene::assignID(Entity* entity)
{
	//Support variables
	int new_entity_ID = 0;
	int new_node_ID = 0;
	bool registered_entity = false;
	vector<int> entity_IDs;
	vector<string> entity_names;
	vector<int> node_IDs;

	//Entities
	ObjectEntity* object;
	LightEntity* light;
	SoundEntity* sound;

	switch (entity->entity_type)
	{
	case(Entity::EntityType::OBJECT):

		//Downcast
		object = (ObjectEntity*)entity;

		//Fill ID lists
		for (auto i = objects.begin(); i != objects.end(); i++)
		{
			//Current Object
			ObjectEntity* current_object = *i;

			//Check whether the entity is already registered
			if (object->name == current_object->name)
			{
				object->object_id = current_object->object_id;
				registered_entity = true;
			}

			//Fill vectors
			if (!registered_entity) entity_IDs.push_back(current_object->object_id);
			node_IDs.push_back(current_object->node_id);
		}

		//Find and assign an available object ID
		if (!registered_entity)
		{
			while (find(entity_IDs.begin(), entity_IDs.end(), new_entity_ID) != entity_IDs.end()) new_entity_ID++;
			object->object_id = new_entity_ID;
		}

		//Find an available node ID
		while (find(node_IDs.begin(), node_IDs.end(), new_node_ID) != node_IDs.end()) new_node_ID++;

		//Assign the new node IDs
		object->node_id = new_node_ID;

		break;
	case(Entity::EntityType::LIGHT):

		//Downcast
		light = (LightEntity*)entity;

		//Fill ID list
		for (auto i = lights.begin(); i != lights.end(); ++i)
		{
			//Current Light
			LightEntity* current_light = *i;

			//Check whether the entity is already registered
			if (light->name == current_light->name)
			{
				light->light_id = current_light->light_id;
				registered_entity = true;
			}

			//Fill vector
			if (!registered_entity) entity_IDs.push_back(current_light->light_id);
		}

		//Find and assign an available light ID
		if (!registered_entity)
		{
			while (find(entity_IDs.begin(), entity_IDs.end(), new_entity_ID) != entity_IDs.end()) new_entity_ID++;
			light->light_id = new_entity_ID;
		}

		break;
	case(Entity::EntityType::SOUND):

		//Downcast
		sound = (SoundEntity*)entity;

		//Fill ID list
		for (auto i = sounds.begin(); i != sounds.end(); ++i)
		{
			//Current Sound
			SoundEntity* current_sound = *i;

			//Check whether the entity is already registered
			if (sound->name == current_sound->name)
			{
				sound->sound_id = current_sound->sound_id;
				registered_entity = true;
			}

			if (!registered_entity) entity_IDs.push_back(current_sound->sound_id);
		}

		//Find and assign an available sound ID
		if (!registered_entity)
		{
			while (find(entity_IDs.begin(), entity_IDs.end(), new_entity_ID) != entity_IDs.end()) new_entity_ID++;
			sound->sound_id = new_entity_ID;
		}

		break;
	}
}

void Scene::assignRelation(ObjectEntity* parent_object, vector<ObjectEntity*> children)
{
	//Set parent object parent to NULL
	parent_object->parent = NULL;

	//Iterate over the children objects
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		//Current object
		ObjectEntity* children_object = *i;

		//Assign children to parent and parent to children
		parent_object->children.push_back(children_object);
		children_object->parent = parent_object;

		//Assign children ID to parent (JSON support)
		parent_object->children_ids.push_back(children_object->node_id);

	}
}

Vector3 Scene::testCollisions(Vector3 currPos, Vector3 deltaPos, float elapsed_time)
{
	Vector3 coll;
	Vector3 collnorm;
	Vector3 nextPos = currPos + deltaPos;

	if (hasCollision(nextPos, coll, collnorm)) {
		Vector3 push_away = normalize(coll - nextPos) * elapsed_time;
		nextPos = currPos - push_away;
		//Vector3 velocity = reflect(Vector3(1,0,1), collnorm) * 0.95;
	};

	return Vector3(nextPos.x, currPos.y, nextPos.z);

}

bool Scene::hasCollision(Vector3 pos, Vector3& coll, Vector3& collnorm) {
	for (size_t i = 0; i < objects.size(); i++)
	{
		ObjectEntity* object = objects[i];
		if (objects[i]->mesh->testSphereCollision(object->model, pos, 20.0f, coll, collnorm)) {
			return true;
		}
			
	}
	return false;
}

//Given a current camera position, returns the object type of the object entity that has in front
ObjectEntity::ObjectType Scene::getCollectable() { 
	//Selected entity and maximum distance of selection
	ObjectEntity::ObjectType type = ObjectEntity::ObjectType::RENDER_OBJECT;
	ObjectEntity* collectable = NULL;
	float max_distance = 500.f;

	//Get global variables
	Camera* camera = main_character->camera;
	Game* game = Game::instance;
	Vector2 mouse_pos = Vector2(game->window_width / 2, game->window_height / 2);
	
	//Compute the direction form mouse to window
	Vector3 ray_direction = camera->getRayDirection(mouse_pos.x, mouse_pos.y, game->window_width, game->window_height);
	Vector3 ray_origin = camera->eye;

	//Search for the object with a Ray Collision in the scene and then return it
	for (size_t i = 0; i < objects.size(); i++)
	{
		//Current object entity
		ObjectEntity* entity = objects[i];

		//Entity properties
		Vector3 entity_position;
		Vector3 entity_normal;

		//Ray collision test
		if (entity->type != ObjectEntity::ObjectType::RENDER_OBJECT && entity->mesh->testRayCollision(entity->model, ray_origin, ray_direction, entity_position, entity_normal, max_distance)) {
			float entity_distance = (entity_position - ray_origin).length();
			if (entity_distance < max_distance)
			{
				//cout << entity->name << " removed succesfully." << endl;
				type = entity->type;
				collectable = entity;
			}
		}
		

	}
	if (collectable && type != ObjectEntity::ObjectType::RENDER_OBJECT)
		removeEntity(collectable);
	
	return type;
}

//True if a collectable can be grabbed
bool Scene::collectableInRange() {
	//Selected entity and maximum distance of selection
	float max_distance = 500.f;

	//Get global variables
	Camera* camera = main_character->camera;
	Game* game = Game::instance;
	Vector2 mouse_pos = Vector2(game->window_width / 2, game->window_height / 2);

	//Compute the direction form mouse to window
	Vector3 ray_direction = camera->getRayDirection(mouse_pos.x, mouse_pos.y, game->window_width, game->window_height);
	Vector3 ray_origin = camera->eye;

	//Search for the object with a Ray Collision in the scene and then return it
	for (size_t i = 0; i < objects.size(); i++)
	{
		//Current object entity
		ObjectEntity* entity = objects[i];

		//Entity properties
		Vector3 entity_position;
		Vector3 entity_normal;

		//Ray collision test
		if (entity->type != ObjectEntity::ObjectType::RENDER_OBJECT && entity->mesh->testRayCollision(entity->model, ray_origin, ray_direction, entity_position, entity_normal, max_distance))
		{
			return true;

		}

	}
	return false;

}

//True if a collectable can be grabbed
bool Scene::hasDoorInRange() {
	//Selected entity and maximum distance of selection
	float max_distance = 500.f;

	//Get global variables
	Camera* camera = main_character->camera;
	Game* game = Game::instance;
	Vector2 mouse_pos = Vector2(game->window_width / 2, game->window_height / 2);

	//Compute the direction form mouse to window
	Vector3 ray_direction = camera->getRayDirection(mouse_pos.x, mouse_pos.y, game->window_width, game->window_height);
	Vector3 ray_origin = camera->eye;

	//Search for the object with a Ray Collision in the scene and then return it
	for (size_t i = 0; i < objects.size(); i++)
	{
		//Current object entity
		ObjectEntity* entity = objects[i];

		//Entity properties
		Vector3 entity_position;
		Vector3 entity_normal;

		//Ray collision test
		if (entity->name == "door" && entity->mesh->testRayCollision(entity->model, ray_origin, ray_direction, entity_position, entity_normal, max_distance))
		{
			return true;

		}

	}
	return false;

}

//JSON Methods
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

	filename = scene_filepath;

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
		int units = readJSONNumber(object_json, "units", 0);
		if (units)
		{
			for (int i = 0; i < units; ++i)
			{
				ObjectEntity* object = new ObjectEntity();

				object->load(object_json, i);

				objects.push_back(object);
			}
		}
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
		int units = readJSONNumber(light_json, "units", 0);
		if (units)
		{
			for (int i = 0; i < units; ++i)
			{
				LightEntity* light = new LightEntity();
				light->load(light_json, i);
				lights.push_back(light);
			}
		}
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
		int units = readJSONNumber(sound_json, "units", 0);
		if (units)
		{
			for (int i = 0; i < units; i++)
			{
				SoundEntity* sound = new SoundEntity();
				sound->load(sound_json, i);
				sounds.push_back(sound);
			}
		}
	}

	//Object tree
	for (auto i = objects.begin(); i != objects.end(); ++i)
	{
		//Current object and children list
		ObjectEntity* object = *i;
		vector<int> children_ids = object->children_ids;

		if (!children_ids.empty())
			for (auto j = children_ids.begin(); j != children_ids.end(); ++j)
			{
				for (auto k = objects.begin(); k != objects.end(); ++k)
				{
					//Children Object
					ObjectEntity* children_object = *k;

					//Push children to parent object list
					if (children_object->node_id == *j)
						object->children.push_back(*k);
				}
			}
	}

	//free memory
	cJSON_Delete(scene_json);

	return true;
}

bool Scene::save()
{
	std::string content;

	if (!readFile(filename, content))
	{
		cout << "ERROR: The Scene JSON has not been found at: " << filename << endl;
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

	//Monster JSON
	cJSON* monster_json = cJSON_AddObjectToObject(scene_json, "monster");
	monster->save(monster_json);

	//Objects JSON
	map<int, vector<cJSON*>> scene_objects;
	cJSON* objects_json = cJSON_AddArrayToObject(scene_json, "objects");
	for (int i = 0; i < objects.size(); i++)
	{
		//Current Object
		ObjectEntity* object = objects[i];

		//Check whether the object is registered or not
		auto it = scene_objects.find(object->object_id);

		//Object hasn't been registered yet
		if (it == scene_objects.end())
		{
			//Create JSONs
			cJSON* object_json = cJSON_CreateObject();
			cJSON* names_array = cJSON_CreateArray();
			cJSON* visibilities_array = cJSON_CreateArray();
			cJSON* models_array = cJSON_CreateArray();
			cJSON* node_IDs_array = cJSON_CreateArray();
			cJSON* children_IDs_array = cJSON_CreateArray();

			//JSON vectors
			vector<cJSON*> jsons_vector = { object_json,names_array,visibilities_array,models_array, node_IDs_array, children_IDs_array };

			//Save the JSON
			object->save(jsons_vector);

			//Add the JSON
			cJSON_AddItemToArray(objects_json, object_json);

			//Register the object
			scene_objects.emplace(object->object_id, jsons_vector); //Register the new object
		}
		else
		{
			object->updateJSON(it->second); //Modify the object_json to add the visibility and model
		}

	}

	//Lights JSON
	map<int, vector<cJSON*>> scene_lights;
	cJSON* lights_json = cJSON_AddArrayToObject(scene_json, "lights");
	for (int i = 0; i < lights.size(); i++)
	{
		//Current light
		LightEntity* light = lights[i];

		//Check whether the light is registered or not
		auto it = scene_lights.find(light->light_id);

		//Light hasn't been registered yet
		if (it == scene_lights.end())
		{
			//Create JSONs
			cJSON* light_json = cJSON_CreateObject();
			cJSON* names_array = cJSON_CreateArray();
			cJSON* visibilities_array = cJSON_CreateArray();
			cJSON* models_array = cJSON_CreateArray();

			//JSON vectors
			vector<cJSON*> jsons_vector = { light_json,names_array,visibilities_array,models_array };

			//Save the JSON
			light->save(jsons_vector);

			//Add the JSON
			cJSON_AddItemToArray(lights_json, light_json);

			//Register the object
			scene_objects.emplace(light->light_id, jsons_vector); //Register the new object
		}
		else
		{
			light->updateJSON(it->second); //Modify the object_json to add the visibility and model
		}
	}

	//Sounds JSON
	map<int, vector<cJSON*>> scene_sounds;
	cJSON* sounds_json = cJSON_AddArrayToObject(scene_json, "sounds");
	for (int i = 0; i < sounds.size(); i++)
	{
		//Current light
		SoundEntity* sound = sounds[i];

		//Check whether the sound is registered or not
		auto it = scene_sounds.find(sound->sound_id);

		//Sound hasn't been registered yet
		if (it == scene_sounds.end())
		{
			//Create JSONs
			cJSON* sound_json = cJSON_CreateObject();
			cJSON* names_array = cJSON_CreateArray();
			cJSON* visibilities_array = cJSON_CreateArray();
			cJSON* models_array = cJSON_CreateArray();

			//JSON vectors
			vector<cJSON*> jsons_vector = { sound_json,names_array,visibilities_array,models_array };

			//Save the JSON
			sound->save(jsons_vector);

			//Add the JSON
			cJSON_AddItemToArray(sounds_json, sound_json);

			//Register the object
			scene_objects.emplace(sound->sound_id, jsons_vector); //Register the new object
		}
		else
		{
			sound->updateJSON(it->second); //Modify the object_json to add the visibility and model
		}
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
