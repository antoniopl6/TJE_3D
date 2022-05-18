
#ifndef ENTITY_H
#define ENTITY_H

#include "includes.h"
#include "utils.h"

class Entity {
public:
	Entity(Matrix44 model); //constructor
	virtual ~Entity() {}; //destructor

	//some attributes 
	std::string name;
	Matrix44 model;

	//methods overwritten by derived classes 
	virtual void render() {};
	virtual void update(float elapsed_time) {};

	//some useful methods...
	Vector3 getPosition();
};
#endif