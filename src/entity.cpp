#include "entity.h"
#include "scene.h"
#include "game.h"
#include <limits>


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

Matrix44 Entity::getRotation()
{
	return model.getRotationOnly();
}

Vector3 Entity::getScale()
{
	return model.getScale();
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
	this->battery = 75.f;
	this->health = 100;
	this->flashIsOn = true;
	this->num_apples = 0;
	this->num_keys = 0;
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
	if (mesh) writeJSONString(main_json, "mesh", mesh->filename);
	else writeJSONString(main_json, "mesh", "");

	//Material
	if (material)material->save(main_json);
}

void MainCharacterEntity::updateMainCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked)
{
	//Mouse input to rotate the camera
	if (((Input::mouse_state) || mouse_locked)) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
	}

	// Define and boost the speed
	float speed = seconds_elapsed * mouse_speed * 4;
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 1.5; //move faster with left shift

	//Camera front and side vectors
	Vector3 camera_front = Vector3();
	Vector3 camera_side = Vector3();

	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_D))
	{
		camera_front = Vector3(1.f,0.f,1.f) * camera->getFrontVector();
		camera_side = Vector3(-camera_front.z, 0.f, camera_front.x);
	}

	//Estimate next position
	Vector3 position_delta = Vector3();
	if (Input::isKeyPressed(SDL_SCANCODE_W)) position_delta = position_delta + camera_front * speed;
	if (Input::isKeyPressed(SDL_SCANCODE_A)) position_delta = position_delta + camera_side * -speed;
	if (!Input::isKeyPressed(SDL_SCANCODE_LCTRL) && Input::isKeyPressed(SDL_SCANCODE_S)) position_delta = position_delta + camera_front * -speed;
	if (Input::isKeyPressed(SDL_SCANCODE_D)) position_delta = position_delta + camera_side * speed;

	//Check collisions
	Vector3 next_position = Scene::instance->testCollisions(camera->eye, position_delta, seconds_elapsed);

	//Assign new position
	camera->lookAt(next_position, next_position + (camera->center - camera->eye), camera->up);

	//Update flashlight position
	if(!Game::instance->render_editor)
		updateModel();

	if (mouse_locked)
		Input::centerMouse();
}

void MainCharacterEntity::updateModel()
{
	//Translation and rotation
	model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
	model.setFrontAndOrthonormalize(camera->getFrontVector().getInverse());

	//Update flashlight model based on main character model
	flashlight->updateBoundingBox();
	
}

void MainCharacterEntity::updateBoundingBox()
{
	if (mesh) world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

void MainCharacterEntity::update(float elapsed_time)
{	
	//Scene singleton
	Scene* scene = Scene::instance;

	//Battery consumption
	float currTime = Game::instance->time;
	if (!this->flashIsOn || this->battery == 0) {
		scene->battery_off = currTime - scene->battery_time;;
		this->flashIsOn = false;
		light->visible = false;
	}
	else if (this->flashIsOn) {
		scene->battery_time = currTime - scene->battery_off;
		if (scene->battery_time > scene->battery_life) {
			this->battery = max(this->battery - scene->battery_reduction, 0);
			scene->battery_time = 0;
			scene->battery_off = currTime;
		}
	}

	//Time that the player is invulnerable
	if (isHitted) {
		if (currTime - playerHittedTime > 3.0f) {
			isHitted = false;
		}
	}

	//With time, the player recovers health
	if (currTime - scene->last_recovery_health > 9.0f) {
		scene->last_recovery_health = currTime;
		health = min(100, health + 25);
	}
	
	//Pick object
	if (Input::isKeyPressed(SDL_SCANCODE_E)) {
		ObjectEntity::ObjectType type;
		type = Scene::instance->getCollectable();
		if (type == ObjectEntity::ObjectType::PICK_OBJECT_KEY)
			num_keys++;
		if (type == ObjectEntity::ObjectType::PICK_OBJECT_BATTERY)
			this->battery += 35.f;
		if (type == ObjectEntity::ObjectType::PICK_OBJECT_APPLE)
			num_apples++;
	}

	//Turn on/off the flashlight
	if (Input::wasKeyPressed(SDL_SCANCODE_F) && battery > 0) 
	{
		this->flashIsOn = !this->flashIsOn;
		light->visible = !light->visible;

	}
	
}

void MainCharacterEntity::print()
{
	cout << "Main character" << endl << endl;
	cout << "Mesh: " << mesh->filename << endl;
	cout << "Apples: " << num_apples << endl;
	cout << "Battery: " << battery << "%" << endl;
	cout << "Keys: " << num_keys << endl;
	cout << "Health: " << health << "%" << endl;
	cout << "Light: " << (flashIsOn ? "On" : "Off") << endl;
	cout << endl;
}

//Monster
MonsterEntity::MonsterEntity()
{
	this->name = "";
	this->visible = true;
	this->mesh = new Mesh();
	this->material = new Material();
	this->bounding_box_trigger = true; //Set it to true for the first iteration
	
	////////////////////////// route define

	idx = 0;
	isInPathRoute = false;
	std::vector<Vector3> points;
	Vector3* p0 = new Vector3(0, 0, 0);
	Vector3* p1 = new Vector3(1600, 0, 0);
	Vector3* p2 = new Vector3(1600, 0, 1600);
	Vector3* p3 = new Vector3(0, 0, 2000);
	Vector3* p4 = new Vector3(1000, 0, 2500);
	Vector3* p5 = new Vector3(0, 0, 7000);
	Vector3* p6 = new Vector3(2000, 0, 1600);
	Vector3* p7 = new Vector3(0, 0, 4000);
	points.push_back(*p0);
	points.push_back(*p1);
	points.push_back(*p2);
	points.push_back(*p3);
	points.push_back(*p4);
	points.push_back(*p5);
	points.push_back(*p6);
	points.push_back(*p7);
	route = new Route(100, 100, points);

	/////////////////////////
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
	if (route == NULL) {
		
	}
}

void MonsterEntity::print()
{
	cout << "Monster" << endl << endl;
	cout << "Mesh: " << mesh->filename << endl;
	cout << endl;
}

bool MonsterEntity::isInFollowRange(MainCharacterEntity* mainCharacter)
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();
	Vector3 toTarget = model.getTranslation() - mainCharacter->camera->eye;

	float dist = toTarget.length();
	toTarget.normalize();

	float forwardDot = forward.dot(toTarget);

	//If the player is in vision range of the monster then should start following
	if (1900.0f > dist && forwardDot > 0.30f) {
		if (300.0f >= dist && !mainCharacter->isHitted) {
			mainCharacter->health = max(0, mainCharacter->health - 25);
			mainCharacter->isHitted = true;
			mainCharacter->playerHittedTime = Game::instance->time;
			Scene::instance->last_recovery_health = Game::instance->time;
		}
		return true;
	}
	return false;
}

void MonsterEntity::updateFollow(float elapsed_time, Camera* camera) //Running animation
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();
	Vector3 toTarget = model.getTranslation() - camera->eye;

	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);
	float rotSpeed = 80.0f;
	float runSpeed = 400.0f;

	//Translate the model of the monster to catch the player
	if (dist > 300) {
		Vector3 translate = forward * -runSpeed * elapsed_time;
		Vector3 monsterPos = Vector3(model.getTranslation().x, 231, model.getTranslation().z);
		Vector3 nextPos = Scene::instance->testCollisions(monsterPos, translate, elapsed_time);
		Vector3 translation = nextPos - model.getTranslation();
		model.translateGlobal(translation.x, 0, translation.z);
	}
	//Change the rotation based on main character pos
	if (forwardDot < 0.98f) {
		model.rotate(rotSpeed * elapsed_time * DEG2RAD * sign(sideDot), Vector3(0, 1, 0));
	}

	this->updateBoundingBox();

}

void MonsterEntity::followPath(float elapsed_time) //Iddle / walking animation
{
	if (!isInPathRoute) { //If monster do not have a route to follow generate one on path closestPoint
		closestPoint = route->getClosestPoint(model.getTranslation());
		Vector2 start = route->getGridVector(model.getTranslation().x + bounding, model.getTranslation().y, model.getTranslation().z + bounding);
		Point currentPoint = Point(start.x, start.y);
		closestPoint->SetPath(route->grid, currentPoint.startx, currentPoint.starty, route->W, route->H);
		isInPathRoute = true;

	}
	else {
		Vector2 newPos = closestPoint->path[idx];
		Vector3 newTranslate = route->getSceneVector(newPos.x, newPos.y);
		if (moveToTarget(elapsed_time, newTranslate))
			idx++;
		if (idx == closestPoint->path_steps) {
			isInPathRoute = false;
			idx = 0;
		}
	}

}

//Returns true if has arrived to pos target, false otherwise
bool MonsterEntity::moveToTarget(float elapsed_time, Vector3 pos)
{
	float rotSpeed = 80.0f;
	float walkSpeed = 200.0f;

	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();
	Vector3 toTarget = model.getTranslation() - pos;

	float dist = toTarget.length();
	toTarget.normalize();
	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);
	Vector3 translate = forward * -walkSpeed * elapsed_time;

	float degrees = computeDegrees(Vector2(forward.x, forward.z), Vector2(toTarget.x, toTarget.z));

	model.rotate(degrees * sign(sideDot), Vector3(0, 1, 0));
	model.translateGlobal(translate.x, 0, translate.z);

	this->updateBoundingBox();

	if (dist <= bounding) {
		return true;

	}
	
	return false;
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
	this->type = RENDER_OBJECT;

	//Object tree
	this->node_id = -1;
	parent = NULL;
}

Matrix44 ObjectEntity::computeGlobalModel()
{
	if (parent)
		return this->model * parent->computeGlobalModel();
	else if (name == "flashlight")
		return this->model * Scene::instance->main_character->model;
	else
		return this->model;
}

void ObjectEntity::updateBoundingBox()
{
	if (mesh) world_bounding_box = transformBoundingBox(this->computeGlobalModel(), this->mesh->box);
}

void ObjectEntity::load(cJSON* object_json, int object_index)
{
	//Object ID
	object_id = readJSONNumber(object_json, "Object_ID", object_id);

	//Name
	cJSON* name_json = readJSONArrayItem(object_json, "names", object_index);
	if (name_json) name = name_json->valuestring;

	//Visibility
	cJSON* visibility_json = readJSONArrayItem(object_json, "visibilities", object_index);
	if (visibility_json) visible = visibility_json->valueint;

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
	if (node_ID_json) node_id = node_ID_json->valueint;

	//Children IDs
	cJSON* children_IDs_json = readJSONArrayItem(object_json, "children_ID", object_index);
	if (children_IDs_json) populateJSONIntArray(children_IDs_json, children_ids);

	//Type
	type = (ObjectType)readJSONNumber(object_json, "Object_type", type);

	//flashlight
	if (name == "flashlight") Scene::instance->main_character->flashlight = this;
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
	cJSON_AddIntVectorToArray(children_IDs_array, &children_ids[0], children_ids.size());

	//Type
	writeJSONNumber(object_json, "Object_type", type);
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

void ObjectEntity::print()
{
	cout << "Object" << endl << endl;
	cout << "Object ID: " << object_id << endl;
	cout << "Name: " << name << endl;
	cout << "Visible: " << visible << endl;
	cout << "Type: " << type << endl;
	cout << "Mesh: " << mesh->filename << endl;
	cout << "Node ID: " << node_id << endl;
	cout << endl;
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
	this->intensity = 5;
	this->max_distance = 1000;

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
	light_id = readJSONNumber(light_json, "Light_ID", light_id);

	//Name
	cJSON* names_json = readJSONArrayItem(light_json, "names", light_index);
	if (names_json) name = names_json->valuestring;

	//Visibility
	cJSON* visibility_json = readJSONArrayItem(light_json, "visibilities", light_index);
	if (visibility_json) visible = visibility_json->valueint;

	//Model
	cJSON* model_json = readJSONArrayItem(light_json, "models", light_index);
	if (model_json) populateJSONFloatArray(model_json, model.m, 16);

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

	//flashlight
	if (name == "flashlight") Scene::instance->main_character->light = this;
}

void LightEntity::save(vector<cJSON*> json)
{
	//JSONs
	cJSON* light_json = json[0];
	cJSON* names_array = json[1];
	cJSON* visibilities_array = json[2];
	cJSON* models_array = json[3];

	//Light ID
	writeJSONNumber(light_json, "Light_ID", light_id);

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

void LightEntity::print()
{
	cout << "Light" << endl << endl;
	cout << "Light ID" << light_id << endl;
	cout << "Name: " << name << endl;
	cout << "Visibility: " << visible << endl;
	cout << "Type: " << light_type << endl;
	cout << "Color: " << color.x << " " << color.y << " " << color.z << endl;
	cout << "Intensity: " << intensity << endl;
	cout << "Cast shadows: " << ((cast_shadows == true) ? "yes" : "no") << endl;
	cout << "Shadow bias: " << shadow_bias << endl;
	
	switch (light_type)
	{
	case(LightType::SPOT_LIGHT):
		cout << "Max distance: " << max_distance << endl;
		cout << "Cone angle: " << cone_angle << endl;
		cout << "Cone exponent: " << cone_exp << endl;
		break;
	case(LightType::DIRECTIONAL_LIGHT):
		cout << "Area size: " << area_size << endl;
		break;
	}

	cout << endl;
}

//Sounds
SoundEntity::SoundEntity()
{
	this->sound_id = -1;
	this->radius = 500.f;
	this->name = "";
	this->visible = true;
	this->model = Matrix44();
	this->entity_type = EntityType::SOUND;
	this->filename = "";
	this->audio = new Audio();
}

void SoundEntity::Play()
{
	audio->Play(filename.c_str());
}

void SoundEntity::Stop()
{
	audio->Stop(audio->sample);
}

void SoundEntity::changeVolume(float volume)
{
	this->volume = volume;
}

void SoundEntity::changeArea(float area)
{
	this->radius = area;
}

void SoundEntity::load(cJSON* sound_json, int sound_index)
{
	//Light ID
	sound_id = readJSONNumber(sound_json, "Sound_ID", sound_id);

	//Name
	cJSON* name_json = readJSONArrayItem(sound_json, "names", sound_index);
	if (name_json) name = name_json->valuestring;

	//Visibility
	cJSON* visibility_json = readJSONArrayItem(sound_json, "visibilities", sound_index);
	if (visibility_json) visible = visibility_json->valueint;

	//Model
	cJSON* model_json = readJSONArrayItem(sound_json, "models", sound_index);
	if (model_json) populateJSONFloatArray(model_json, model.m, 16);

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
	writeJSONNumber(sound_json, "Sound_ID", sound_id);

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

void SoundEntity::update(float elapsed_time)
{
	//TODO
}

void SoundEntity::print()
{
	cout << "Sound" << endl << endl;
	cout << "Sound ID: " << sound_id << endl;
	cout << "Name: " << name << endl;
	cout << "Active: " << (visible ? "yes" : "no") << endl;
	cout << "Volume: " << volume << endl;
	cout << "Radius: " << radius << endl;
	cout << endl;
}
