#include "entity.h"
#include "scene.h"

//Entity
Entity::Entity() {
	name = "";
	visible = true;
	model = Matrix44();
}

Vector3 Entity::getPosition()
{
	return model.getTranslation();
}

//Main character
MainCharacterEntity::MainCharacterEntity() {
	this->name = "";
	this->visible = true;
	this->model = Matrix44();
	this->entity_type = EntityType::MAIN;
	this->camera = new Camera();
	this->mesh = new Mesh();
	this->material = new Material();
	this->bounding_box_trigger = true; //Set it to true for the first iteration
}

void MainCharacterEntity::updateMainCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked)
{
	//Mouse input to rotate the camera
	if (((Input::mouse_state) || mouse_locked)) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
	}

	//Turn around
	if (Input::wasKeyPressed(SDL_SCANCODE_Q))
		camera->center *= -1.f;
	if (Input::isKeyPressed(SDL_SCANCODE_Q))
		return;

	// Define and boost the speed
	float speed = seconds_elapsed * mouse_speed * 4;
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 1.5; //move faster with left shift

	//Camera front and side vectors
	Vector3 camera_front = Vector3();
	Vector3 camera_side = Vector3();

	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_D))
	{
		camera_front = ((camera->center - camera->eye) * Vector3(1.f, 0.f, 1.f)).normalize();
		camera_side = Vector3(-camera_front.z, 0.f, camera_front.x);
	}

	Vector3 nextPos = Vector3();
	if (Input::isKeyPressed(SDL_SCANCODE_W)) nextPos = nextPos + camera_front * speed;
	if (Input::isKeyPressed(SDL_SCANCODE_A)) nextPos = nextPos + camera_side * -speed;
	if (!Input::isKeyPressed(SDL_SCANCODE_LCTRL) && Input::isKeyPressed(SDL_SCANCODE_S)) nextPos = nextPos + camera_front * -speed;
	if (Input::isKeyPressed(SDL_SCANCODE_D)) nextPos = nextPos + camera_side * speed;
	nextPos = Scene::instance->testCollisions(camera->eye, nextPos, seconds_elapsed);
	camera->lookAt(nextPos, nextPos + (camera->center - camera->eye), camera->up);
	//To navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

void MainCharacterEntity::updateBoundingBox()
{
	if (mesh) world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

void MainCharacterEntity::load(cJSON* main_json)
{
	//General features
	name = readJSONString(main_json, "name", name.c_str());
	visible = readJSONBoolean(main_json, "visible", visible);

	//Model
	vector<float> main_model;
	readJSONFloatVector(main_json, "model", main_model);
	for (int i = 0; i < main_model.size(); ++i) model.m[i] = main_model[i];

	//Mesh
	string mesh_path = readJSONString(main_json, "mesh", "");
	if (!mesh_path.empty())
		mesh = Mesh::Get(mesh_path.c_str());
	else
		cout << "ERROR: Main character mesh hasn't been found at: " << mesh_path << endl;
	
	//Material
	if (!mesh_path.empty())
	{
		Material* material = Material::Get(mesh_path.c_str());
		if (material)
			this->material = material;
		else
		{
			cJSON* material_json = cJSON_GetObjectItemCaseSensitive(main_json, "material");
			this->material->load(material_json);
			this->material->registerMaterial(mesh_path.c_str());
		}
	}
}

void MainCharacterEntity::save(cJSON* main_json)
{
	//General features
	writeJSONString(main_json, "name", name.c_str());
	writeJSONBoolean(main_json, "visible", visible);
	writeJSONFloatVector(main_json, "model", model.m, 16);

	//Mesh
	if(mesh) writeJSONString(main_json, "mesh", mesh->filename);
	else writeJSONString(main_json, "mesh", "");

	//Material
	if (material)material->save(main_json);
}

void MainCharacterEntity::update(float elapsed_time)
{

}

//Monster
MonsterEntity::MonsterEntity()
{
	this->name = "";
	this->visible = true;
	this->mesh = new Mesh();
	this->material = new Material();
	this->bounding_box_trigger = true; //Set it to true for the first iteration
}

void MonsterEntity::updateBoundingBox()
{
	if (mesh) world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

void MonsterEntity::load(cJSON* monster_json)
{
	//General features
	name = readJSONString(monster_json, "name", name.c_str());
	visible = readJSONBoolean(monster_json, "visible", visible);

	//Model
	vector<float> monster_model;
	readJSONFloatVector(monster_json, "model", monster_model);
	for (int i = 0; i < monster_model.size(); ++i) model.m[i] = monster_model[i];

	//Mesh
	string mesh_path = readJSONString(monster_json, "mesh", "");
	if (!mesh_path.empty())
	{
		mesh = Mesh::Get(mesh_path.c_str());
	}
	else
		cout << "ERROR: Monster mesh hasn't been found at: " << mesh_path << endl;

	//Material
	if (!mesh_path.empty())
	{
		Material* material = Material::Get(mesh_path.c_str());
		if (material)
			this->material = material;
		else
		{
			cJSON* material_json = cJSON_GetObjectItemCaseSensitive(monster_json, "material");
			this->material->load(material_json);
			this->material->registerMaterial(mesh_path.c_str());
		}
	}
}

void MonsterEntity::save(cJSON* monster_json)
{
	//General features
	writeJSONString(monster_json, "name", name.c_str());
	writeJSONBoolean(monster_json, "visible", visible);
	writeJSONFloatVector(monster_json, "model", model.m, 16);

	//Mesh
	if (mesh) writeJSONString(monster_json, "mesh", mesh->filename);
	else writeJSONString(monster_json, "mesh", "");

	//Material
	if (material)material->save(monster_json);
}

void MonsterEntity::update(float elapsed_time)
{

}

float sign(float num) {
	return num >= 0.0f ? 1.0f : -1.0f;
}

bool MonsterEntity::isInFollowRange(Camera* camera)
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();
	Vector3 toTarget = model.getTranslation() - camera->eye;

	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);

	//If the player is in vision range of the monster then should start following
	return (1900.0f > dist && forwardDot > 0.30f);
}

void MonsterEntity::updateFollow(float elapsed_time, Camera* camera)
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();
	Vector3 toTarget = model.getTranslation() - camera->eye;

	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);
	float speed = 80.0f;

	if (dist > 400.0f) {
		Vector3 translate = forward * -speed * elapsed_time;
		model.translate(translate.x, translate.y, translate.z);
		//model.translate(-toTarget.x * speed * elapsed_time, 0, -toTarget.z * speed * elapsed_time);

	}
	//Change the rotation based on main character pos
	if (forwardDot < 0.98f) {
		model.rotate(speed * elapsed_time * DEG2RAD * sign(sideDot), Vector3(0, 1, 0));
	}

	this->updateBoundingBox();

}

//Objects
ObjectEntity::ObjectEntity() {
	this->object_id = -1;
	this->name = "";
	this->visible = true;
	this->model = Matrix44();
	this->entity_type = EntityType::OBJECT;
	this->mesh = new Mesh();
	this->material = new Material();
	this->bounding_box_trigger = true; //Set it to true for the first iteration
	this->node_id = -1;
}

Matrix44 ObjectEntity::computeGlobalMatrix()
{
	if (parent)
		return parent->computeGlobalMatrix() * this->model;
	else
		return this->model;
}

void ObjectEntity::updateBoundingBox()
{
	if(mesh) world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

void ObjectEntity::load(cJSON* object_json, int object_index)
{
	//Object ID
	object_id = readJSONNumber(object_json, "Object ID", object_id);
	
	//Name
	cJSON* name_json = readJSONArrayItem(object_json, "names", object_index);
	if(name_json) name = name_json->valuestring;

	//Visibility
	cJSON* visibility_json = readJSONArrayItem(object_json, "visibilities", object_index);
	if(visibility_json) visible = visibility_json->valueint;

	//Model
	cJSON* model_json = readJSONArrayItem(object_json, "models", object_index);
	populateJSONFloatArray(model_json, model.m, 16);

	//Mesh
	string mesh_path = readJSONString(object_json, "mesh", "");
	if (!mesh_path.empty())
		mesh = Mesh::Get(mesh_path.c_str());
	else
		cout << "ERROR: " << name << " mesh hasn't been found at: " << mesh_path << endl;

	//Material
	if (!mesh_path.empty())
	{
		Material* material = Material::Get(mesh_path.c_str());
		if (material)
			this->material = material;
		else
		{
			cJSON* material_json = cJSON_GetObjectItemCaseSensitive(object_json, "material");
			this->material->load(material_json);
			this->material->registerMaterial(mesh_path.c_str());
		}
	}

	//Node ID
	cJSON* node_ID_json = readJSONArrayItem(object_json, "node_ID", object_index);
	if(node_ID_json) object_id = node_ID_json->valueint;

	//Children IDs
	cJSON* children_IDs_json = readJSONArrayItem(object_json, "children_ID", object_index);
	if(children_IDs_json) populateJSONIntArray(children_IDs_json, children_ids);
}

void ObjectEntity::save(vector<cJSON*> json)
{
	//JSONs
	cJSON* object_json = json[0];
	cJSON* names_array = json[1];
	cJSON* visibilities_array = json[2];
	cJSON* models_array = json[3];
	cJSON* node_IDs_array = json[4];
	cJSON* children_IDs_array = json[5];

	//Object ID
	writeJSONNumber(object_json, "Object_ID", object_id);

	//Units
	writeJSONNumber(object_json, "units", 1);

	//Name
	cJSON_AddItemToObject(object_json, "names", names_array);
	cJSON_AddStringToArray(names_array, name.c_str());

	//Visibility
	cJSON_AddItemToObject(object_json, "visibilities", visibilities_array);
	cJSON_AddBoolToArray(visibilities_array, visible);

	//Model
	cJSON_AddItemToObject(object_json, "models", models_array);
	cJSON_AddFloatVectorToArray(models_array, model.m, 16);

	//Mesh
	if (mesh) writeJSONString(object_json, "mesh", mesh->filename);
	else writeJSONString(object_json, "mesh", "");

	//Material
	if (material)material->save(object_json);

	//Node ID
	cJSON_AddItemToObject(object_json, "node_ID", node_IDs_array);
	cJSON_AddNumberToArray(node_IDs_array, node_id);

	//Children IDs
	cJSON_AddItemToObject(object_json, "children_ID", children_IDs_array);
	cJSON_AddIntVectorToArray(object_json, &children_ids[0], children_ids.size());
}

void ObjectEntity::updateJSON(vector<cJSON*> json)
{
	//JSONs
	cJSON* object_json = json[0];
	cJSON* names_array = json[1];
	cJSON* visibilities_array = json[2];
	cJSON* models_array = json[3];
	cJSON* node_IDs_array = json[4];
	cJSON* children_IDs_array = json[5];

	//Increase the number of units
	int units = readJSONNumber(object_json, "units", 0);
	if (units)
	{
		units++;
		replaceJSONNumber(object_json, "units", units);
	}	

	//Add name
	cJSON_AddStringToArray(names_array, name.c_str());

	//Add visibility
	cJSON_AddBoolToArray(visibilities_array, visible);

	//Add model
	cJSON_AddFloatVectorToArray(models_array, model.m, 16);

	//Add node ID
	cJSON_AddNumberToArray(node_IDs_array, node_id);

	//Add children IDs
	cJSON_AddIntVectorToArray(children_IDs_array, &children_ids[0], children_ids.size());

}

void ObjectEntity::update(float elapsed_time)
{

}

//Lights
LightEntity::LightEntity() 
{
	//General features
	this->light_id = -1;
	this->name = "";
	this->visible = true;
	this->entity_type = LIGHT;
	this->light_type = LightType::POINT_LIGHT;
	this->color.set(1.0f, 1.0f, 1.0f);
	this->intensity = 1;
	this->max_distance = 100;

	//Spot light
	this->cone_angle = 45;
	this->cone_exp = 30;

	//Directional light
	this->area_size = 1000;

	//Shadows
	this->cast_shadows = false;
	this->shadow_bias = 0.001;
	this->shadow_camera = NULL;
}

void LightEntity::load(cJSON* light_json, int light_index)
{
	//Light ID
	light_id = readJSONNumber(light_json, "Light ID", light_id);

	//Name
	cJSON* names_json = readJSONArrayItem(light_json, "names", light_index);
	if(names_json) name = names_json->valuestring;

	//Visibility
	cJSON* visibility_json = readJSONArrayItem(light_json, "visibilities", light_index);
	if(visibility_json) visible = visibility_json->valueint;

	//Model
	cJSON* model_json = readJSONArrayItem(light_json, "models", light_index);
	if(model_json) populateJSONFloatArray(model_json, model.m, 16);
	
	//General features
	color = readJSONVector3(light_json, "color", color);
	intensity = readJSONNumber(light_json, "intensity", intensity);
	max_distance = readJSONNumber(light_json, "max_distance", max_distance);
	std::string type_field = readJSONString(light_json, "light_type", "");

	//Light features
	if (type_field == "POINT_LIGHT") {
		light_type = LightType::POINT_LIGHT;
	}
	else if (type_field == "SPOT_LIGHT") {
		light_type = LightType::SPOT_LIGHT;
		cone_angle = readJSONNumber(light_json, "cone_angle", cone_angle);
		cone_exp = readJSONNumber(light_json, "cone_exp", cone_exp);
	}
	else if (type_field == "DIRECTIONAL_LIGHT") {
		light_type = LightType::DIRECTIONAL_LIGHT;
		area_size = readJSONNumber(light_json, "area_size", area_size);
	}
	else
	{
		cout << "ERROR: Light type " << type_field << " unknown" << endl;
	}

	//Shadow features
	cast_shadows = readJSONBoolean(light_json, "cast_shadows", cast_shadows);
	shadow_bias = readJSONNumber(light_json, "shadow_bias", shadow_bias);
}

void LightEntity::save(vector<cJSON*> json)
{
	//JSONs
	cJSON* light_json = json[0];
	cJSON* names_array = json[1];
	cJSON* visibilities_array = json[2];
	cJSON* models_array = json[3];

	//Light ID
	writeJSONNumber(light_json, "Light ID", light_id);

	//Units
	writeJSONNumber(light_json, "units", 1);

	//Name
	cJSON_AddItemToObject(light_json, "names", names_array);
	cJSON_AddStringToArray(names_array, name.c_str());

	//Visibility
	cJSON_AddItemToObject(light_json, "visibilities", visibilities_array);
	cJSON_AddBoolToArray(visibilities_array, visible);

	//Model
	cJSON_AddItemToObject(light_json, "models", models_array);
	cJSON_AddFloatVectorToArray(models_array, model.m, 16);

	//General features
	writeJSONVector3(light_json, "color", this->color);
	writeJSONNumber(light_json, "intensity", this->intensity);
	writeJSONNumber(light_json, "max_distance", this->max_distance);

	//Specific features
	switch (this->light_type)
	{
	case(LightType::POINT_LIGHT):
		writeJSONString(light_json, "light_type", "POINT_LIGHT");
		break;
	case(LightType::SPOT_LIGHT):
		writeJSONNumber(light_json, "cone_angle", this->cone_angle);
		writeJSONNumber(light_json, "cone_exp", this->cone_exp);
		writeJSONBoolean(light_json, "cast_shadows", this->cast_shadows);
		writeJSONNumber(light_json, "shadow_bias", this->shadow_bias);
		writeJSONString(light_json, "light_type", "SPOT_LIGHT");
		break;
	case(LightType::DIRECTIONAL_LIGHT):
		writeJSONNumber(light_json, "area_size", this->area_size);
		writeJSONBoolean(light_json, "cast_shadows", this->cast_shadows);
		writeJSONString(light_json, "light_type", "DIRECTIONAL_LIGHT");
		break;
	}
}

void LightEntity::updateJSON(vector<cJSON*> json)
{
	//JSONs
	cJSON* light_json = json[0];
	cJSON* visibilities_array = json[1];
	cJSON* models_array = json[2];

	//Increase the number of units
	int units = readJSONNumber(light_json, "units", 0);
	if (units)
	{
		units++;
		replaceJSONNumber(light_json, "units", units);
	}

	//Add visibility
	cJSON_AddBoolToArray(visibilities_array, visible);

	//Add model
	cJSON_AddFloatVectorToArray(models_array, model.m, 16);
}

void LightEntity::update(float elapsed_time)
{

}

//Sounds
SoundEntity::SoundEntity()
{
	this->sound_id = -1;
	this->name = "";
	this->visible = true;
	this->model = Matrix44();
	this->entity_type = EntityType::SOUND;
	this->filename = "";
}

void SoundEntity::load(cJSON* sound_json, int sound_index)
{
	//Light ID
	sound_id = readJSONNumber(sound_json, "Sound ID", sound_id);

	//Name
	cJSON* name_json = readJSONArrayItem(sound_json, "names", sound_index);
	if(name_json) name = name_json->valuestring;

	//Visibility
	cJSON* visibility_json = readJSONArrayItem(sound_json, "visibilities", sound_index);
	if(visibility_json) visible = visibility_json->valueint;

	//Model
	cJSON* model_json = readJSONArrayItem(sound_json, "models", sound_index);
	if(model_json) populateJSONFloatArray(model_json, model.m, 16);
	
	//Filename
	filename = readJSONString(sound_json, "filename", "");

}

void SoundEntity::save(vector<cJSON*> json)
{
	//JSONs
	cJSON* sound_json = json[0];
	cJSON* names_array = json[1];
	cJSON* visibilities_array = json[2];
	cJSON* models_array = json[3];

	//Sound ID
	writeJSONNumber(sound_json, "Sound ID", sound_id);

	//Units
	writeJSONNumber(sound_json, "units", 1);

	//Name
	cJSON_AddItemToObject(sound_json, "names", names_array);
	cJSON_AddStringToArray(names_array, name.c_str());

	//Visibility
	cJSON_AddItemToObject(sound_json, "visibilities", visibilities_array);
	cJSON_AddBoolToArray(visibilities_array, visible);

	//Model
	cJSON_AddItemToObject(sound_json, "models", models_array);
	cJSON_AddFloatVectorToArray(models_array, model.m, 16);

	//Filename
	writeJSONString(sound_json, "filename", this->filename);
}

void SoundEntity::updateJSON(vector<cJSON*> json)
{
	//JSONs
	cJSON* sound_json = json[0];
	cJSON* visibilities_array = json[1];
	cJSON* models_array = json[2];

	//Increase the number of units
	int units = readJSONNumber(sound_json, "units", 0);
	if (units)
	{
		units++;
		replaceJSONNumber(sound_json, "units", units);
	}

	//Add visibility
	cJSON_AddBoolToArray(visibilities_array, visible);

	//Add model
	cJSON_AddFloatVectorToArray(models_array, model.m, 16);
}


