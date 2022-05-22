#include "entity.h"


Vector3 Entity::getPosition()
{
	return model.getTranslation();
}

MainCharacterEntity::MainCharacterEntity(string name, Matrix44 model, Camera* camera, Mesh* mesh, Texture* texture) {
	/*assert((mesh != null), "Null mesh given");
	assert(texture != null, "Null texture given");*/
	this->name = name;
	this->model = model;
	this->entity_type = EntityType::MAIN;
	this->camera = camera;
	this->mesh = mesh;
	this->texture = texture;
	this->bounding_box_trigger = false;

	updateBoundingBox();
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

	//Move the camera along with the main character
	if (Input::isKeyPressed(SDL_SCANCODE_W)) camera->lookAt(camera->eye + camera_front * speed, camera->center + camera_front * speed, camera->up);
	if (Input::isKeyPressed(SDL_SCANCODE_A)) camera->lookAt(camera->eye - camera_side * speed, camera->center - camera_side * speed, camera->up);
	if (Input::isKeyPressed(SDL_SCANCODE_S)) camera->lookAt(camera->eye - camera_front * speed, camera->center - camera_front * speed, camera->up);
	if (Input::isKeyPressed(SDL_SCANCODE_D)) camera->lookAt(camera->eye + camera_side * speed, camera->center + camera_side * speed, camera->up);

	//To navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

void MainCharacterEntity::updateBoundingBox()
{
	world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

//Monster

//Objects
ObjectEntity::ObjectEntity(string name, Matrix44 model, Mesh* mesh, Texture* texture) {
	/*assert(mesh != null, "Null mesh given");
	assert(texture != null, "Null texture given");*/
	this->name = name;
	this->model = model;
	this->entity_type = EntityType::OBJECT;
	this->mesh = mesh;
	this->texture = texture;
	this->bounding_box_trigger = false;

	updateBoundingBox();
}

void ObjectEntity::updateBoundingBox()
{
	world_bounding_box = transformBoundingBox(this->model, this->mesh->box);
}

//Lights
LightEntity::LightEntity(string name, Vector3 color, float intensity, float max_distance, int light_type, float cone_angle, float cone_exp, float area_size, bool cast_shadows, float shadow_bias)
{
	this->name = name;
	this->entity_type = EntityType::LIGHT;
	this->color = color;
	this->intensity = intensity;
	this->max_distance = max_distance;
	this->light_type = (LightType)light_type;
	this->cone_angle = cone_angle;
	this->cone_exp = cone_exp;
	this->area_size = area_size;
	this->cast_shadows = cast_shadows;
	this->shadow_bias = shadow_bias;
	this->shadow_camera = NULL;
}

MonsterEntity::MonsterEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, BoundingBox bounding, BoundingBox boundingCatchRange, BoundingBox boundingViewRange, Matrix44 model) {
	/*assert(mesh != null, "Null mesh given");
	assert(texture != null, "Null texture given");
	assert(shader != null, "Null shader given");*/
	this->model = model;
	this->mesh = mesh;
	this->texture = texture;
	this->shader = shader;
	this->color = color;
	this->bounding = bounding;
	this->boundingCatchRange = boundingCatchRange;
	this->boundingViewRange = boundingViewRange;
}

PickEntity::PickEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, Matrix44 model, PickType type) {
	/*assert(mesh != null, "Null mesh given");
	assert(texture != null, "Null texture given");
	assert(shader != null, "Null shader given");*/
	this->model = model;
	this->mesh = mesh;
	this->texture = texture;
	this->shader = shader;
	this->color = color;
	this->type = type;
}

SoundEntity::SoundEntity(Audio audio, Matrix44 model) {
	this->audio = audio;
	this->model = model;
}

void MonsterEntity::render(){
	if (isRunning) {
		//Render animation of running monster
		
	}
	else {


	}


}

bool PickEntity::isInPickRange(Matrix44 mainModel) {
	return false;
}