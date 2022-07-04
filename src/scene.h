#ifndef SCENE_H
#define SCENE_H
//En esta clase se almacenarán todas las entidades en la escena y contendrá metodos para facilitar el trabajo en ellas
//Usa el patrón singleton

#pragma once
#include "utils.h"
#include "includes.h"
#include "entity.h"
#include "camera.h"
#include "shader.h"
#include "path.h"

//Forward declaration
class FBO;

using namespace std;

class Scene
{
public:

	//Singleton
	static Scene* instance;

	//General features
	string filename;
	Vector3 ambient_light;
	Camera* main_camera;
	Shader* shader;
	float battery_time = 0.0f; //Handles battery consumption
	float battery_life = 2.5f; // The time that the battery reduction has before it's spent
	float battery_reduction = 5.0f;
	float battery_off = 0.0f; //Time the battery is on off state
	float last_recovery_health = 0.0f; //Handles recovery of health by time

	//Scene shadows
	FBO* fbo; //Frame Buffer Object
	Texture* shadow_atlas; //Shadow map of the lights of the scene

	//Entities
	MainCharacterEntity* main_character;
	MonsterEntity* monster;
	vector<ObjectEntity*> objects;
	vector<LightEntity*> lights;
	vector<SoundEntity*> sounds;

	//Path for monster
	vector<Route*> route;

	//Counters
	int num_objects;
	int num_lights;
	int num_sounds;
	int num_shadows;

	//Scene properties
	bool show_atlas;
	int atlas_scope;

	//Scene triggers


	//Constructor
	Scene();

	//Entity methods
	void clear();
	void addEntity(Entity* entity);
	void removeEntity(Entity* entity);
	void assignID(Entity* entity);
	void assignRelation(ObjectEntity* parent, vector<ObjectEntity*> children);
	Vector3 testCollisions(Vector3 currPos, Vector3 nexPos, float elapsed_time);

	bool hasCollision(Vector3 pos, Vector3& coll, Vector3& collnorm);
	bool hasDoorInRange();
	ObjectEntity::ObjectType getCollectable();
	bool collectableInRange();

	//JSON methods
	bool load(const char* scene_filepath);
	bool save();

};


#endif