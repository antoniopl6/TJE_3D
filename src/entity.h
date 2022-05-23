
#ifndef ENTITY_H
#define ENTITY_H

#pragma once
#include "includes.h"
#include "utils.h"
#include "input.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "framework.h"

using namespace std;

enum EntityType {
	MAIN = 0,
	MONSTER = 1,
	OBJECT = 2,
	LIGHT = 3,
	SOUND = 4
};

enum ObjectType {
	PICK_OBJECT = 1,
	RENDER_OBJECT = 2,
};

enum LightType {
	POINT_LIGHT = 0,
	SPOT_LIGHT = 1,
	DIRECTIONAL_LIGHT = 2
};


class Entity {
public:
	Entity() {}; //constructor
	virtual ~Entity() {}; //destructor

	//Entity features
	string name;
	Matrix44 model;
	EntityType entity_type;

	//Methods overwritten by derived classes 
	virtual void update(float elapsed_time) {};
	virtual void load(cJSON* entity_json) {};
	virtual void save(cJSON* entity_json) {};
	virtual void updateBoundingBox() {};

	//Some useful methods...
	Vector3 getPosition();
};

class MainCharacterEntity: public Entity {
public:
	
	//Main features
	Camera* camera;
	Mesh* mesh;
	Texture* texture;
	BoundingBox world_bounding_box;

	//Triggers
	bool bounding_box_trigger;

	MainCharacterEntity(); //constructor

	//Methods
	void updateMainCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked);

	//Inherited methods
	virtual void updateBoundingBox() override;
	virtual void load(cJSON* main_json) override;
	virtual void save(cJSON* main_json) override;
	virtual void update(float elapsed_time) override;
};

class MonsterEntity : public Entity {
public:

	//Monster features
	Mesh* mesh;
	Texture* texture;
	BoundingBox world_bounding_box;

	//Methods
	MonsterEntity();

	//Inherited methods
	virtual void updateBoundingBox() override;
	virtual void load(cJSON* monster_json) override;
	virtual void save(cJSON* monster_json) override;
	virtual void update(float elapsed_time) override;
};

class ObjectEntity : public Entity {
public:
	
	Mesh* mesh;
	Texture* texture;
	BoundingBox world_bounding_box;

	//Triggers
	bool bounding_box_trigger;

	ObjectEntity(); //constructor

	//Inherited methods
	virtual void updateBoundingBox() override;
	virtual void load(cJSON* object_json) override;
	virtual void save(cJSON* object_json) override;
	virtual void update(float elapsed_time) override;

};

class LightEntity : public Entity {
public:

	//Light features
	Vector3 color;
	float intensity;
	float max_distance;
	LightType light_type;

	//Spot Light
	float cone_angle;
	float cone_exp;

	//Directional Light
	float area_size;

	//Shadows
	bool cast_shadows;
	float shadow_bias;
	Camera* shadow_camera;

	//Constructor
	LightEntity(); 

	//Inherited methods
	virtual void load(cJSON* light_json) override;
	virtual void save(cJSON* light_json) override;
	virtual void update(float elapsed_time) override;
};

class SoundEntity : public Entity{
public:	
	string filename;

	//Methods
	SoundEntity();

	//JSON methods
	virtual void load(cJSON* sound_json) override;
	virtual void save(cJSON* sound_json) override;
};

#endif