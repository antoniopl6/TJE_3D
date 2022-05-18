#include "entity.h"

Entity::Entity(Matrix44 model)
{
	this->model = model;
}

Vector3 Entity::getPosition()
{
	return model.getTranslation();
}


MonsterEntity::MonsterEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, BoundingBox bounding, BoundingBox boundingCatchRange, BoundingBox boundingViewRange, Matrix44 model) : Entity::Entity(model) {
	/*assert(mesh != null, "Null mesh given");
	assert(texture != null, "Null texture given");
	assert(shader != null, "Null shader given");*/
	this->mesh = mesh;
	this->texture = texture;
	this->shader = shader;
	this->color = color;
	this->bounding = bounding;
	this->boundingCatchRange = boundingCatchRange;
	this->boundingViewRange = boundingViewRange;
}

PickEntity::PickEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, Matrix44 model, PickType type) : Entity::Entity(model) {
	/*assert(mesh != null, "Null mesh given");
	assert(texture != null, "Null texture given");
	assert(shader != null, "Null shader given");*/
	this->mesh = mesh;
	this->texture = texture;
	this->shader = shader;
	this->color = color;
	this->type = type;
}

SoundEntity::SoundEntity(Audio audio, Matrix44 model) : Entity::Entity(model) {
	this->audio = audio;

}

void MonsterEntity::render(){
	if (isRunning) {
		//Render animation of running monster
		
	}
	else {


	}


}

bool PickEntity::isInPickRange(Matrix44 mainModel) {

}