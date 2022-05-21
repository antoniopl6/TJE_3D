#ifndef SCENE_H
#define SCENE_H
//En esta clase se almacenar�n todas las entidades en la escena y contendr� metodos para facilitar el trabajo en ellas
//Usa el patr�n singleton

#pragma once //Import things once
#include "utils.h"
#include "includes.h"
#include "camera.h"
#include "game.h"
#include "entity.h"

class Scene
{
public:

	//Singleton
	static Scene* instance;

	//General features
	Vector3 ambient_light;
	Camera* main_camera;

	//Scene shadows
	FBO* fbo; //Frame Buffer Object
	Texture* shadow_atlas; //Shadow map of the lights of the scene

	//Entities
	Entity* main_character;
	MonsterEntity* monster;

	std::vector<Entity*> objects;
	std::vector<Entity*> lights;
	std::vector<SoundEntity*> sounds;

	//Scene triggers
	bool entity_trigger; //Triggers if an entity has changed his visibility or a visible entity has changed its model.
	bool shadow_visibility_trigger; //Triggers changes in shadow casting or light visibility for lights that cast shadows.

	//Methods
	Scene();
	void clear();
	void addEntity(Entity*);
	void load();


};


#endif