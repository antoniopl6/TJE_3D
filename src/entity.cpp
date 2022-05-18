#include "entity.h"

Entity::Entity(Matrix44 model)
{
	this->model = model;
}

Vector3 Entity::getPosition()
{
	return model.getTranslation();
}


