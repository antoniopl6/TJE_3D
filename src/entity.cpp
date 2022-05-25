#include "entity.h"
#include "scene.h"

//Entity
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
	if (Input::isKeyPressed(SDL_SCANCODE_S)) nextPos = nextPos + camera_front * -speed;
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
	readJSONVector(main_json, "model", main_model);
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

	//Model
	cJSON* main_model = cJSON_CreateFloatArray(this->model.m, 16);
	cJSON_AddItemToObject(main_json, "model", main_model);

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
	readJSONVector(monster_json, "model", monster_model);
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

	//Model
	cJSON* monster_model = cJSON_CreateFloatArray(this->model.m, 16);
	cJSON_AddItemToObject(monster_json, "model", monster_model);

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

bool MonsterEntity::isInFollowRange(Camera * camera)
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
	//Change the rotation based on main character pos
	if (dist > 400.0f) {
		std::cout << dist << std::endl;
		Vector3 translate = forward * speed * elapsed_time;
		model.translate(-translate.x, -translate.y, -translate.z);
		//model.translate(-toTarget.x * speed * elapsed_time, 0, -toTarget.z * speed * elapsed_time);
		this->updateBoundingBox();

	}
	if (forwardDot < 0.98f) {
		model.rotate(speed * elapsed_time * DEG2RAD * sign(sideDot), Vector3(0, 1, 0));
	}
}

//Objects
ObjectEntity::ObjectEntity() {
	this->name = "";
	this->visible = true;
	this->model = Matrix44();
	this->entity_type = EntityType::OBJECT;
	this->mesh = new Mesh();
	this->material = new Material();
	this->bounding_box_trigger = true; //Set it to true for the first iteration
}

void ObjectEntity::updateBoundingBox()
{
	if(mesh) world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

void ObjectEntity::load(cJSON* object_json)
{
	//General features
	name = readJSONString(object_json, "name", name.c_str());
	visible = readJSONBoolean(object_json, "visible", visible);

	//Model
	vector<float> object_model;
	readJSONVector(object_json, "model", object_model);
	for (int i = 0; i < object_model.size(); ++i) model.m[i] = object_model[i];

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
}

void ObjectEntity::save(cJSON* object_json)
{
	//General features
	writeJSONString(object_json, "name", name.c_str());
	writeJSONBoolean(object_json, "visible", visible);
	
	//Model
	cJSON* object_model = cJSON_CreateFloatArray(this->model.m, 16);
	cJSON_AddItemToObject(object_json, "model", object_model);
	
	//Mesh
	if(mesh) writeJSONString(object_json, "mesh", mesh->filename);
	else writeJSONString(object_json, "mesh", "");
	
	//Material
	if (material)material->save(object_json);

}

void ObjectEntity::update(float elapsed_time)
{

}

//Lights
LightEntity::LightEntity() 
{
	//General features
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

void LightEntity::load(cJSON* light_json)
{
	//General features
	name = readJSONString(light_json, "name", name.c_str());
	visible = readJSONBoolean(light_json, "visible", visible);
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

	//Model
	vector<float> light_model;
	readJSONVector(light_json, "model", light_model);
	for (int i = 0; i < light_model.size(); ++i) model.m[i] = light_model[i];
}

void LightEntity::save(cJSON* light_json)
{
	//General features
	writeJSONString(light_json, "name", this->name.c_str());
	writeJSONBoolean(light_json, "visible", visible);
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

	//Model
	cJSON* light_model = cJSON_CreateFloatArray(this->model.m, 16);
	cJSON_AddItemToObject(light_json, "model", light_model);
}

void LightEntity::update(float elapsed_time)
{

}

//Sounds
SoundEntity::SoundEntity()
{
	this->name = "";
	this->visible = true;
	this->model = Matrix44();
	this->entity_type = EntityType::SOUND;
	this->filename = "";
}

void SoundEntity::load(cJSON* sound_json)
{
	//General features
	name = readJSONString(sound_json, "name", "");
	visible = readJSONBoolean(sound_json, "visible", visible);
	filename = readJSONString(sound_json, "filename", "");

	//Model
	vector<float> sound_model;
	readJSONVector(sound_json, "model", sound_model);
	for (int i = 0; i < sound_model.size(); ++i) model.m[i] = sound_model[i];
}

void SoundEntity::save(cJSON* sound_json)
{
	//General features
	writeJSONString(sound_json, "name", this->name);
	writeJSONBoolean(sound_json, "visible", visible);
	writeJSONString(sound_json, "filename", this->filename);

	//Model
	cJSON* sound_model = cJSON_CreateFloatArray(this->model.m, 16);
	cJSON_AddItemToObject(sound_json, "model", sound_model);
}
