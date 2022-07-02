#pragma once
#include "scene.h"
#include "mesh.h"
#include "texture.h"
#include "cMTL.h"

using namespace std;

class Editor3D
{
public:

	//Enums
	enum CameraType
	{
		MAIN,
		ENTITY
	};

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

	enum Actions {
		
		//Model actions
		TRANSLATE,
		ROTATE,
		SCALE,	

		//Light actions
		COLOR,
		INTENSITY,
		MAX_DISTANCE,
		CONE_ANGLE
	};

	//Game variables
	Scene* scene;

	//Camera
	Camera* camera;
	CameraType current_camera;
	bool camera_focus;

	//MTL Parser
	cMTL* Parser;

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

	//Action variables
	Actions current_action;
	float translation_speed;
	float rotation_speed;
	float scale_speed;
	float color_speed;
	float intensity_speed;
	float distance_speed;
	float cone_speed;

	//Constructor
	Editor3D(Scene* scene);

	//Edit
	ObjectEntity* selected_object = NULL;
	LightEntity* selected_light = NULL;
	SoundEntity* selected_sound = NULL;

	//General Methods
	void reset(); //Resets class flowtime
	void show(); //
	void work();
	void render();

	//Actions methods
	void addEntity();
	void editEntity(Entity* entity);
	void removeEntity(Entity* entity);
	ObjectEntity* selectEntity();
	void placeEntity(Entity* entity);

	//Camera methods
	void switchCamera();
	void focusCamera(Entity* entity);
	void updateCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked);


};