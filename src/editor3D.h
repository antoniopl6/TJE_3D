#pragma once
#include "game.h"

using namespace std;

enum EditorLayer {
	LAYER1,
	LAYER2,
	LAYER3
};

enum MenuOption {
	NONE,
	ADD,
	EDIT,
	REMOVE
};

enum EntityOption
{
	NONE,
	OBJECT,
	LIGHT,
	SOUND
};

class Editor3D
{
public:
	
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
	void reset();
	void show();
	void work();
	void render();

	//Actions
	void addEntity();
	void editEntity(Entity* entity);
	void removeEntity(Entity* entity);

};
