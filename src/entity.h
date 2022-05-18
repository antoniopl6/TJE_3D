
#ifndef ENTITY_H
#define ENTITY_H

#include "includes.h"
#include "utils.h"

enum EntityType {
	MAIN = 0,
	MONSTER = 1,
	OBJECT = 2,
	LIGHT = 3
};

enum LightType {
	POINT = 0,
	SPOT = 1,
	DIRECTIONAL = 2
};

enum ObjectType {
	PICK_OBJECT = 1,
	RENDER_OBJECT = 2,
};

class Entity {
public:
	Entity(Matrix44 model); //constructor
	virtual ~Entity() {}; //destructor

	//some attributes 
	EntityType entity_type;
	std::string name;
	Matrix44 model;

	//methods overwritten by derived classes 
	virtual void update(float elapsed_time) {};

	//some useful methods...
	Vector3 getPosition();
};
#endif