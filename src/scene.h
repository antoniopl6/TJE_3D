#ifndef SCENE_H
#define SCENE_H
//En esta clase se almacenarán todas las entidades en la escena y contendrá metodos para facilitar el trabajo en ellas
//Usa el patrón singleton

#pragma once //Import things once
#include "utils.h"
#include "includes.h"
#include "camera.h"
#include "entity.h"
#include "shader.h"

class Scene
{
public:

	//Singleton
	static Scene* instance;

	//General features
	Vector3 ambient_light;
	Camera* main_camera;
	Shader* shader;

	//Scene shadows
	FBO* fbo; //Frame Buffer Object
	Texture* shadow_atlas; //Shadow map of the lights of the scene

	//Entities
	MainCharacterEntity* main_character;
	MonsterEntity* monster;
	std::vector<ObjectEntity*> objects;
	std::vector<LightEntity*> lights;
	std::vector<SoundEntity*> sounds;
	int num_objects;
	int num_lights;

	//Scene triggers
	bool camera_trigger; //Triggers if the camera has moved in the space.
	bool entity_trigger; //Triggers if an entity has changed his visibility or a visible entity has changed its model.
	bool shadow_visibility_trigger; //Triggers changes in shadow casting or light visibility for lights that cast shadows.

	//Methods
	Scene(Camera* camera);
	void clear();
	void load();
	void addEntity(Entity* entity);
	void AllocateMemory();


};


#endif