#include "editor3D.h"
#include "game.h"
#include <filesystem>

//Constructor
Editor3D::Editor3D(Scene* scene)
{
	//Game instance
	Game* game = Game::instance;

	//Game Scene
	this->scene = scene;
	
	//Camera
	camera = new Camera();
	camera->setPerspective(45.f, game->window_width / (float)game->window_height, 1.0f, camera->far_plane + 10000.f);
	current_camera = MAIN;

	//MTL Parser
	Parser = new cMTL(scene);

	//Intializate vectors
	string assets_path = filesystem::current_path().string() + "\\data\\assets";
	string sounds_path = filesystem::current_path().string() + "\\data\\sounds";

	for (const auto& entry : filesystem::directory_iterator(assets_path))
	{
		string str_path = entry.path().string();
		string asset = str_path.substr(assets_path.size() + 1, str_path.size());
		if(asset != "monster" && asset != "main_character")
			assets.push_back(asset);
	};

	for (const auto& entry : filesystem::directory_iterator(sounds_path))
	{
		string str_path = entry.path().string();
		string sound = str_path.substr(sounds_path.size() + 1, str_path.size());
		sounds.push_back(sound);
	};

	lights.reserve(3);
	lights[0] = "point light";
	lights[1] = "spot light";
	lights[2] = "directional light";

	//Vector support variables
	assets_size = assets.size();
	lights_size = lights.size();
	sounds_size = sounds.size();

	//Bool support variables
	start_menu = true;

	//Options
	current_layer = EditorLayer::LAYER1;
	menu_option = MenuOption::NO_MENU;
	entity_option = EntityOption::NO_ENTITY;

	//Selectors
	if (assets_size)
		current_asset = 0;
	if (lights_size)
		current_light = 0;
	if (sounds_size)
		current_sound = 0;

	//Action variables
	current_action = Actions::TRANSLATE;
	translation_speed = 1.f;
	rotation_speed = 1.f;
	scale_speed = 1.001;
}

//General methods
void Editor3D::reset()
{
	//Bool support variables
	start_menu = true;

	//Option
	current_layer = EditorLayer::LAYER1;
	menu_option = MenuOption::NO_MENU;
	entity_option = EntityOption::NO_ENTITY;

	//Selectors
	if (assets_size)
		current_asset = 0;
	if (lights_size)
		current_light = 0;
	if (sounds_size)
		current_sound = 0;

	//Action variables
	current_action = Actions::TRANSLATE;
}

void Editor3D::show()
{
	//Support boolean variables
	bool keyboard_interaction = Input::wasKeyPressed(SDL_SCANCODE_1) || Input::wasKeyPressed(SDL_SCANCODE_2) || Input::wasKeyPressed(SDL_SCANCODE_3) || Input::wasKeyPressed(SDL_SCANCODE_TAB);
	bool exit_interaction = Input::wasKeyPressed(SDL_SCANCODE_ESCAPE);
	bool selection_status = (!selected_object && !selected_light && !selected_sound);
	bool menu_change = ((start_menu || keyboard_interaction) && selection_status) || exit_interaction;

	//Select Menu Option
	if (current_layer == EditorLayer::LAYER1) {
		if (Input::wasKeyPressed(SDL_SCANCODE_1))
			menu_option = MenuOption::ADD, current_layer = EditorLayer::LAYER2;
		if (Input::wasKeyPressed(SDL_SCANCODE_2))
			menu_option = MenuOption::EDIT, current_layer = EditorLayer::LAYER2;
		if (Input::wasKeyPressed(SDL_SCANCODE_3))
			menu_option = MenuOption::REMOVE, current_layer = EditorLayer::LAYER2;
	}

	//Select Entity Option
	else if (current_layer == EditorLayer::LAYER2) {
		if (Input::wasKeyPressed(SDL_SCANCODE_1))
			entity_option = EntityOption::OBJECT, current_layer = EditorLayer::LAYER3;
		if (Input::wasKeyPressed(SDL_SCANCODE_2))
		{
			entity_option = EntityOption::LIGHT;
			current_layer = EditorLayer::LAYER3;
			if (menu_option != MenuOption::ADD) switchCamera();
		}
			
		if (Input::wasKeyPressed(SDL_SCANCODE_3))
		{
			entity_option = EntityOption::SOUND;
			current_layer = EditorLayer::LAYER3;
			if (menu_option != MenuOption::ADD) switchCamera();
		}
			
	}

	//Exit options
	if (Input::wasKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		if (current_layer == EditorLayer::LAYER2)
		{
			menu_option = MenuOption::NO_MENU;
			current_layer = EditorLayer::LAYER1;
			start_menu = true;
		}
		else if (current_layer == EditorLayer::LAYER3)
		{
			if(selected_object)
				selected_object = NULL;
			else if(selected_light)
				selected_light = NULL;
			else if(selected_sound)
				selected_sound = NULL;
			else
			{
				entity_option = EntityOption::NO_ENTITY;
				current_layer = EditorLayer::LAYER2;
			}
			if (current_camera == ENTITY) switchCamera();

		}

	}

	//Selectors
	if (Input::wasKeyPressed(SDL_SCANCODE_TAB))
	{
		if (menu_option == MenuOption::ADD && entity_option == EntityOption::OBJECT)
		{
			int next_asset = current_asset + 1;
			if (next_asset < assets_size) current_asset = next_asset;
			else current_asset = 0;
		}
		else if (entity_option == EntityOption::LIGHT)
		{
			if (menu_option == MenuOption::ADD)
			{
				int next_light = current_light + 1;
				if (next_light < lights_size) current_light = next_light;
				else current_light = 0;
			}
			else if (menu_option == MenuOption::EDIT || menu_option == MenuOption::REMOVE)
			{
				int next_light = current_light + 1;
				if (next_light < scene->lights.size()) current_light = next_light;
				else current_light = 0;
			}

		}
		else if (entity_option == EntityOption::SOUND)
		{
			if (menu_option == MenuOption::ADD)
			{
				int next_sound = current_sound + 1;
				if (next_sound < sounds_size) current_sound = next_sound;
				else current_sound = 0;
			}
			else if (menu_option == MenuOption::EDIT || menu_option == MenuOption::REMOVE)
			{
				int next_sound = current_sound + 1;
				if (next_sound < scene->sounds.size()) current_sound = next_sound;
				else current_sound = 0;
			}

		}


	}

	//Menu display
	if (menu_change)
	{
		switch (menu_option)
		{
		case(MenuOption::ADD):
			switch (entity_option)
			{
			case(EntityOption::OBJECT):
				cout << "Select an asset from the list" << endl << endl;
				for (int i = 0; i < assets_size; ++i)
				{
					if (i == current_asset)
						cout << assets[i] << " <--" << endl;
					else
						cout << assets[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::LIGHT):
				cout << "Select a light from the list" << endl << endl;
				for (int i = 0; i < lights_size; ++i)
				{
					if (i == current_light)
						cout << lights[i] << " <--" << endl;
					else
						cout << lights[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::SOUND):
				cout << "Select a sound from the list" << endl << endl;
				for (int i = 0; i < sounds_size; ++i)
				{
					if (i == current_sound)
						cout << sounds[i] << " <--" << endl;
					else
						cout << sounds[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::NO_ENTITY):
				cout << "Select an entity type" << endl << endl;
				cout << "1. Object" << endl;
				cout << "2. Light" << endl;
				cout << "3. Sound" << endl << endl;
				break;
			}
			break;
		case(MenuOption::EDIT):
			switch (entity_option)
			{
			case(EntityOption::OBJECT):
				cout << "Click an object to start editing it" << endl << endl;
				break;
			case(EntityOption::LIGHT):
				cout << "Select a light from the list" << endl << endl;
				for (int i = 0; i < scene->lights.size(); ++i)
				{
					if (i == current_light)
					{
						cout << scene->lights[i] << " <--" << endl;
						focusCamera(scene->lights[i]);
					}
					else
						cout << scene->lights[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::SOUND):
				cout << "Select a sound from the list" << endl << endl;
				for (int i = 0; i < scene->sounds.size(); ++i)
				{
					if (i == current_sound)
					{
						cout << scene->sounds[i] << " <--" << endl;
						focusCamera(scene->sounds[i]);
					}
					else
						cout << scene->sounds[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::NO_ENTITY):
				cout << "Select an entity type" << endl << endl;
				cout << "1. Object" << endl;
				cout << "2. Light" << endl;
				cout << "3. Sound" << endl << endl;
				break;
			}
			break;
		case(MenuOption::REMOVE):
			switch (entity_option)
			{
			case(EntityOption::OBJECT):
				cout << "Click an object to remove it" << endl << endl;
				break;
			case(EntityOption::LIGHT):
				cout << "Select a light from the list" << endl << endl;
				for (int i = 0; i < scene->lights.size(); ++i)
				{
					if (i == current_light)
					{
						cout << scene->lights[i] << " <--" << endl;
						focusCamera(scene->lights[i]);
					}
					else
						cout << scene->lights[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::SOUND):
				cout << "Select a sound from the list" << endl << endl;
				for (int i = 0; i < scene->sounds.size(); ++i)
				{
					if (i == current_sound)
					{
						cout << scene->sounds[i] << " <--" << endl;
						focusCamera(scene->sounds[i]);
					}
					else
						cout << scene->sounds[i] << endl;
				}
				cout << endl;
				break;
			case(EntityOption::NO_ENTITY):
				cout << "Select an entity type" << endl << endl;
				cout << "1. Object" << endl;
				cout << "2. Light" << endl;
				cout << "3. Sound" << endl << endl;
				break;
			}
			break;
		case(MenuOption::NO_MENU):
			cout << endl << "Welcome to the editor mode" << endl << endl;
			cout << "1. Add entity" << endl;
			cout << "2. Edit entity" << endl;
			cout << "3. Remove entity" << endl << endl;
			start_menu = false;
			break;
		}
	}

	//Action mode
	if (selected_object || selected_light || selected_sound)
	{
		if (Input::wasKeyPressed(SDL_SCANCODE_1))
		{
			current_action = Actions::TRANSLATE;
			cout << "Action changed to TRANSLATE" << endl << endl;
		}
		else if (Input::wasKeyPressed(SDL_SCANCODE_2))
		{
			current_action = Actions::ROTATE;
			cout << "Action changed to ROTATE" << endl << endl;
		}
		else if (Input::wasKeyPressed(SDL_SCANCODE_3))
		{
			current_action = Actions::SCALE;
			cout << "Action changed to SCALE" << endl << endl;
		}
	}

}

void Editor3D::work()
{
	//Add entity
	if (menu_option == MenuOption::ADD && Input::wasMousePressed(SDL_BUTTON_RIGHT))
	{
		addEntity();
	}
	
	//Edit entity
	else if (menu_option == MenuOption::EDIT)
	{

		if (entity_option == EntityOption::OBJECT)
		{			
			//Pick an entity with a ray cast
			if (!selected_object && Input::wasMousePressed(SDL_BUTTON_RIGHT))
			{
				selected_object = selectEntity();

				if (selected_object)
				{
					//Show the selected entity in the screen
					cout << "Selected Entity: " << endl << "\tName: " << selected_object->name << endl << "\tID: " << selected_object->object_id << endl << endl;

					//Change camera view
					switchCamera();
					focusCamera(selected_object);
				}
			}
			
			//Send that entity to editEntity method
			if (selected_object)
				editEntity(selected_object);
				
		}
		else if (entity_option == EntityOption::LIGHT)//&& (Input::mouse_state == SDL_BUTTON_MIDDLE))
		{
			if (Input::wasKeyPressed(SDL_SCANCODE_SPACE))
				selected_light = scene->lights[current_light];

			if (selected_light != NULL)
				editEntity(scene->lights[current_light]);
		}
		else if (entity_option == EntityOption::SOUND)
		{
			if (Input::wasKeyPressed(SDL_SCANCODE_SPACE))
				selected_sound = scene->sounds[current_sound];

			if (selected_sound != NULL)
				editEntity(selected_sound);
		}
	}

	//Remove entity
	else if (menu_option == MenuOption::REMOVE)
	{
		if (entity_option == EntityOption::OBJECT && Input::wasMousePressed(SDL_BUTTON_RIGHT))
		{
			//Pick an entity with a ray cast
			ObjectEntity* selected_entity = selectEntity();
			
			//Send that entity to RemoveEntity method
			if (selected_entity)
				removeEntity(selected_entity);
		}
		else if (entity_option == EntityOption::LIGHT && Input::wasKeyPressed(SDL_SCANCODE_SPACE))
		{
			removeEntity(scene->lights[current_light]);
		}
		else if (entity_option == EntityOption::SOUND && Input::wasKeyPressed(SDL_SCANCODE_SPACE))
		{
			removeEntity(scene->sounds[current_sound]);
		}
	}

}

void Editor3D::render()
{
	show();
	work();
}

//Actions methods
void Editor3D::addEntity()
{

	if (entity_option == EntityOption::OBJECT) {
		
		//List of objects
		vector<ObjectEntity*> objects;

		//Parse MTL
		string asset_name = assets[current_asset];
		string root = filesystem::current_path().string() + "\\data\\assets\\" + asset_name;
		objects = Parser->Parse(root, asset_name);

		//Check objects list
		if (objects.empty())
		{
			cout << "ERROR: The object list is empty" << endl << endl;
			return;
		}

		//Iterate over the objects list
		for (auto i = objects.begin(); i != objects.end(); i++)
		{
			//Current object
			ObjectEntity* object = *i;

			//Place parent object
			if (!object->parent)
			{
				//Cast a ray and place the parent in the ray direction
				placeEntity(object);

				//Check if the object is too far away
				Vector3 object_position = object->model.getTranslation();
				Vector3 main_position = scene->main_camera->eye;
				float distance = (object_position - main_position).length();
				if (distance > 800.f)
				{
					cout << "ERROR: The spawn position is too far away, try a closer position" << endl << endl;
					return;
				}
			}

			//Add new objects to the scene
			scene->addEntity(object);
		}

		//Feedback
		cout << "Object succesfully created" << endl << endl;
	}
	else if (entity_option == EntityOption::LIGHT)
	{
		//New light
		LightEntity* new_light = new LightEntity();

		//LightType
		string light_type = lights[current_light];
		if (light_type == "point light")
		{
			new_light->light_type = LightEntity::LightType::POINT_LIGHT;
		}
		else if (light_type == "spot light")
		{
			new_light->light_type = LightEntity::LightType::SPOT_LIGHT;
		}
		else if (light_type == "directional light")
		{
			new_light->light_type = LightEntity::LightType::DIRECTIONAL_LIGHT;
		}

		//Assign ID
		scene->assignID(new_light);

		//Cast a ray and place the entity in the ray direction
		placeEntity(new_light);

		//Add the new light to the scene
		scene->addEntity(new_light);

		//Feedback
		cout << "Light succesfully created" << endl << endl;
	}
	else if (entity_option == EntityOption::SOUND)
	{
		//New sound
		SoundEntity* new_sound = new SoundEntity();

		//Sound filename
		string sound_name = sounds[current_sound];
		new_sound->filename = "\\data\\assets\\" + sound_name;

		//Assign ID
		scene->assignID(new_sound);

		//Cast a ray and place the entity in the ray direction
		placeEntity(new_sound);

		//Add the new sound to the scene
		scene->addEntity(new_sound);

		//Feedback
		cout << "Sound succesfully created" << endl << endl;
	}
}

void Editor3D::editEntity(Entity* entity)
{	
	//Change speed
	switch (current_action)
	{
	case(Actions::TRANSLATE):

		if (Input::isMousePressed(SDL_BUTTON_X1))
			translation_speed = max(translation_speed - 0.1, 0.f);
		else if (Input::isMousePressed(SDL_BUTTON_X2))
			translation_speed += 0.1;
		break;

	case(Actions::ROTATE):

		if (Input::isMousePressed(SDL_BUTTON_X1))
			rotation_speed = max(rotation_speed - 0.1, 0.f);
		else if (Input::isMousePressed(SDL_BUTTON_X2))
			rotation_speed += 0.1;
		break;

	case(Actions::SCALE):
		
		if (Input::isMousePressed(SDL_BUTTON_X1))
			scale_speed = max(scale_speed - 0.0001, 1.001);
		else if (Input::isMousePressed(SDL_BUTTON_X2))
			scale_speed = min(scale_speed + 0.0001, 1.999);
		break;
	}

	//Edit
	switch (current_action)
	{
	case(Actions::TRANSLATE):
		
		//Translate entity with WASDQE
		if (Input::isKeyPressed(SDL_SCANCODE_D))
			entity->model.translate(-translation_speed, 0, 0);
		if (Input::isKeyPressed(SDL_SCANCODE_A))
			entity->model.translate(translation_speed, 0, 0);
		if (Input::isKeyPressed(SDL_SCANCODE_E))
			entity->model.translate(0, translation_speed, 0);
		if (Input::isKeyPressed(SDL_SCANCODE_Q))
			entity->model.translate(0, -translation_speed, 0);
		if (Input::isKeyPressed(SDL_SCANCODE_W))
			entity->model.translate(0, 0, translation_speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S))
			entity->model.translate(0, 0, -translation_speed);
		break;

	case(Actions::ROTATE):
		
		//Rotate entity with WASDQE
		if (Input::isKeyPressed(SDL_SCANCODE_D))
			entity->model.rotate(-rotation_speed * DEG2RAD, Vector3(0, 1, 0));
		if (Input::isKeyPressed(SDL_SCANCODE_A))
			entity->model.rotate(rotation_speed * DEG2RAD, Vector3(0, 1, 0));
		if (Input::isKeyPressed(SDL_SCANCODE_E))
			entity->model.rotate(-rotation_speed * DEG2RAD, Vector3(0, 0, 1));
		if (Input::isKeyPressed(SDL_SCANCODE_Q))
			entity->model.rotate(rotation_speed * DEG2RAD, Vector3(0, 0, 1));
		if (Input::isKeyPressed(SDL_SCANCODE_W))
			entity->model.rotate(-rotation_speed * DEG2RAD, Vector3(1, 0, 0));
		if (Input::isKeyPressed(SDL_SCANCODE_S))
			entity->model.rotate(rotation_speed * DEG2RAD, Vector3(1, 0, 0));
		break;

	case(Actions::SCALE):

		float deescalate_speed = 1.f - (scale_speed - 1.f);
		
		//Scale entity with WASDQE and +/-
		if (Input::isKeyPressed(SDL_SCANCODE_D))
			entity->model.scale(scale_speed, 1.f, 1.f);
		if (Input::isKeyPressed(SDL_SCANCODE_A))
			entity->model.scale(deescalate_speed, 1.f, 1.f);
		if (Input::isKeyPressed(SDL_SCANCODE_E))
			entity->model.scale(1.f, scale_speed, 1.f);
		if (Input::isKeyPressed(SDL_SCANCODE_Q))
			entity->model.scale(1.f, deescalate_speed, 1.f);
		if (Input::isKeyPressed(SDL_SCANCODE_W))
			entity->model.scale(1.f, 1.f, scale_speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S))
			entity->model.scale(1.f, 1.f, deescalate_speed);
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHTBRACKET))
			entity->model.scale(scale_speed, scale_speed, scale_speed);
		if (Input::isKeyPressed(SDL_SCANCODE_SLASH)) //May be SDL_SCANCODE_LEFTBRACKET
			entity->model.scale(deescalate_speed, deescalate_speed, deescalate_speed);
			
		break;
	}
	
	entity->updateBoundingBox();
}

void Editor3D::removeEntity(Entity* entity)
{
	//Remove entity
	scene->removeEntity(entity);

	//Feedback
	cout << entity->name << " succesfully removed" << endl << endl;
}

ObjectEntity* Editor3D::selectEntity() {
	
	//Selected entity and maximum distance of selection
	ObjectEntity* selected_object = NULL;
	float ray_max_distance = 2000.f;
	float object_min_distance = 2000.f;
	
	//Get global variables
	Vector2 mouse = Input::mouse_position;
	Camera* camera = scene->main_character->camera;
	Game* game = Game::instance;

	//Compute the direction form mouse to window
	Vector3 ray_direction = camera->getRayDirection(mouse.x, mouse.y, game->window_width, game->window_height);
	Vector3 ray_origin = camera->eye;
	
	//Search for the object with a Ray Collision in the scene and then return it
	for (size_t i = 0; i < scene->objects.size(); i++)
	{
		//Current object entity
		ObjectEntity* object = scene->objects[i];

		//Entity properties
		Vector3 object_position;
		Vector3 object_normal;

		//Ray collision test
		if (object->mesh->testRayCollision(object->model, ray_origin, ray_direction, object_position, object_normal, ray_max_distance))
		{
			float object_distance = (object_position - ray_origin).length();
			if (object_distance < object_min_distance)
			{
				object_min_distance = object_distance;
				selected_object = object;
			}
		}
	}
	return selected_object;
}

void Editor3D::placeEntity(Entity* entity) {
	
	//Support variables
	Vector2 mouse = Input::mouse_position;
	Camera* camera = scene->main_camera;
	Game* game = Game::instance;
	
	//Compute the direction form mouse to window
	Vector3 ray_direction = camera->getRayDirection(mouse.x, mouse.y, game->window_width, game->window_height);
	Vector3 ray_origin = camera->eye;

	//Obtain collision from camera origin to plane
	Vector3 spawn_position = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), ray_origin, ray_direction);

	//Set translation to the entity
	entity->model.translate(spawn_position.x, spawn_position.y, spawn_position.z);

};

//Camera methods
void Editor3D::switchCamera()
{
	(current_camera == MAIN) ? current_camera = ENTITY : current_camera = MAIN;
}

void Editor3D::focusCamera(Entity* entity)
{
	//Entities
	ObjectEntity* object;
	LightEntity* light;
	SoundEntity* sound;

	//Camera variables
	Vector3 camera_center = Vector3(0.f, 0.f, 0.f);
	Vector3 camera_eye = Vector3(200.f, 200.f, 200.f);
	Vector3 camera_up = Vector3(0.f, 1.f, 0.f);

	switch (entity->entity_type)
	{
	case(Entity::EntityType::OBJECT):
		//Downcast
		object = (ObjectEntity*)entity;

		//Camera variables
		camera_center = object->world_bounding_box.center;
		camera_eye = 200 * object->model.frontVector() + camera_center;
		break;
	case(Entity::EntityType::LIGHT):
		//Downcast
		light = (LightEntity*)entity;

		//Camera variables
		camera_center = light->model.getTranslation();
		camera_eye = 200 * light->model.frontVector() + camera_center;
		break;
	case(Entity::EntityType::SOUND):
		//Downcast
		sound = (SoundEntity*)entity;

		//Camera variables
		camera_center = sound->model.getTranslation();
		camera_eye = 200 * sound->model.frontVector() + camera_center;
		break;
	}

	//Focus camera
	camera->lookAt(camera_eye, camera_center, camera_up);
}

void Editor3D::updateCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked)
{
	float orbit_speed = seconds_elapsed * 0.5;

	//Mouse input to rotate the cam
	if (Input::mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) //move in first person view
	{
		camera->rotate(-Input::mouse_delta.x * orbit_speed * 0.5, Vector3(0, 1, 0));
		Vector3 right = camera->getLocalVector(Vector3(1, 0, 0));
		camera->rotate(-Input::mouse_delta.y * orbit_speed * 0.5, right);
	}

	//Mouse input to orbit the camera around the center
	if (Input::mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
	}

	if (Input::mouse_wheel_trigger)
	{
		camera->changeDistance(Input::mouse_wheel_delta * 0.5);
	}

	//To navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

