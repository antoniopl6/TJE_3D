#ifndef ENTITY_H
#define ENTITY_H

#pragma once
#include "includes.h"
#include "utils.h"
#include "input.h"
#include "camera.h"
#include "mesh.h"
#include "material.h"
#include "framework.h"
#include "animation.h"
#include "audio.h"
#include "path.h"

using namespace std;

class ObjectEntity;
class LightEntity;

class Entity {
public:
	//Entity Enum
	enum EntityType {
		MAIN = 0,
		MONSTER = 1,
		OBJECT = 2,
		LIGHT = 3,
		SOUND = 4
	};

	Entity(); //constructor
	virtual ~Entity() {}; //destructor

	//Entity features
	string name;
	bool visible;
	Matrix44 model;
	EntityType entity_type;

	//Methods overwritten by derived classes 
	virtual void update(float elapsed_time) {};
	virtual void print() {};
	virtual void updateBoundingBox() {};

	//Some useful methods...
	Vector3 getPosition();
	Matrix44 getRotation();
	Vector3 getScale();
};

class MainCharacterEntity : public Entity {
public:

	//Main features
	Camera* camera;
	Mesh* mesh;
	Material* material;
	BoundingBox world_bounding_box;

	//Game values
	int num_apples;
	float battery;
	float health;
	bool flashIsOn;
	ObjectEntity* flashlight;
	LightEntity* light;
	int num_keys;
	bool isHitted = false;
	float playerHittedTime = 0.0f;

	//Triggers
	bool bounding_box_trigger;

	//Constructor
	MainCharacterEntity();

	//Methods
	void updateMainCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked);
	void updateModel();

	//JSON methods
	void load(cJSON* main_json);
	void save(cJSON* main_json);

	//Inherited methods
	virtual void update(float elapsed_time) override;
	virtual void print() override;
	virtual void updateBoundingBox() override;
};

class MonsterEntity : public Entity {
public:

	//Monster features
	Mesh* mesh;
	Material* material;
	BoundingBox world_bounding_box;
	
	//Animations
	Animation* running;
	Animation* walking;
	Animation* idle;

	//Path finding
	Route* route;
	bool isInPathRoute;
	Point* closestPoint;
	float bounding = 7.0f;
	int idx;

	//Triggers
	bool bounding_box_trigger;

	//Bools
	bool isRunning;

	//Constructor
	MonsterEntity();

	//Methods
	bool isInFollowRange(MainCharacterEntity* mainCharacter);
	void updateFollow(float elapsed_time, Camera* camera);
	void followPath(float elapsed_time);
	bool moveToTarget(float elapsed_time, Vector3 pos);

	//JSON Methods
	void load(cJSON* mosnter_json);
	void save(cJSON* monster_json);

	//Inherited methods
	virtual void update(float elapsed_time) override;
	virtual void print() override;
	virtual void updateBoundingBox() override;
	
};

class ObjectEntity : public Entity {
public:
	//Object enum
	enum ObjectType {
		PICK_OBJECT_KEY = 1,
		PICK_OBJECT_APPLE = 2,
		PICK_OBJECT_BATTERY = 3,
		RENDER_OBJECT = 0,
	};

	//Object features
	int object_id;
	Mesh* mesh;
	Material* material;
	BoundingBox world_bounding_box;
	ObjectType type;

	//Object tree
	int node_id;
	ObjectEntity* parent;
	vector<ObjectEntity*> children;
	vector<int> children_ids; //Just for JSON support

	//Triggers
	bool bounding_box_trigger;

	//Constructor
	ObjectEntity();

	//Children methods
	Matrix44 computeGlobalModel();

	//JSON methods
	void load(cJSON* object_json, int object_index);
	void save(vector<cJSON*> json);
	void updateJSON(vector<cJSON*> json);

	//Inherited methods
	virtual void update(float elapsed_time) override;
	virtual void print() override;
	virtual void updateBoundingBox() override;

};

class LightEntity : public Entity {
public:
	//Light enum
	enum LightType {
		POINT_LIGHT = 0,
		SPOT_LIGHT = 1,
		DIRECTIONAL_LIGHT = 2
	};

	//Light features
	int light_id;
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
	int shadow_index;
	bool cast_shadows;
	float shadow_bias;
	Camera* shadow_camera;

	//Constructor
	LightEntity();

	//JSON methods
	void load(cJSON* light_json, int light_index);
	void save(vector<cJSON*> json);
	void updateJSON(vector<cJSON*> json);

	//Inherited methods
	virtual void update(float elapsed_time) override;
	virtual void print() override;
};

class SoundEntity : public Entity {
public:

	//Sound features
	int sound_id;
	float volume;
	float radius;
	string filename;
	Audio* audio;

	//Methods
	SoundEntity();
	void Play();
	void Stop();
	void changeVolume(float volume);
	void changeArea(float area);

	//JSON methods
	void load(cJSON* sound_json, int sound_index);
	void save(vector<cJSON*> json);
	void updateJSON(vector<cJSON*> json);

	//Inherited methods
	virtual void update(float elapsed_time) override;
	virtual void print() override;
};

#endif