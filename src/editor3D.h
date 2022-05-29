#pragma once
#include "scene.h"
#include "mesh.h"
#include "texture.h"

using namespace std;

class Editor3D
{
public:

	//Enums
	enum EditorLayer {
		LAYER1, //Select the menu option (add, remove or edit)
		LAYER2, //Select entity
		LAYER3
	};

	enum MenuOption {
		NO_MENU,
		ADD,
		EDIT,
		REMOVE
	};

	enum EntityOption
	{
		NO_ENTITY,
		OBJECT,
		LIGHT,
		SOUND
	};

	//Game variables
	Scene* scene;

	//Use vectors to store the available resources
	vector<string> assets;
	vector<string> lights;
	vector<string> sounds;

	//Vector support variables
	int assets_size;
	int lights_size;
	int sounds_size;

	//Bool support variables
	bool start_menu;

	//Menu display
	EditorLayer current_layer;
	MenuOption menu_option;
	EntityOption entity_option;

	//Selectors
	int current_asset;
	int current_light;
	int current_sound;

	//Constructor
	Editor3D(Scene* scene);

	//General Methods
	void reset(); //Resets class flowtime
	void show(); //
	void work();
	void render();

	//Actions
	void addEntity();
	void editEntity(Entity* entity);
	void removeEntity(Entity* entity);
	void addEntityInScene(Entity* entity);
	ObjectEntity* selectEntityInScene();
};