#include "editor3D.h"

#include <filesystem>


Editor3D::Editor3D(Scene* scene)
{
	//Game Scene
	this->scene = scene;

	//Intializate vectors
	string assets_path = filesystem::current_path().string() + "\\data\\assets";
	string sounds_path = filesystem::current_path().string() + "\\data\\sounds";
	
	for (const auto& entry : filesystem::directory_iterator(assets_path))
	{
		string str_path = entry.path().string();
		string asset = str_path.substr(assets_path.size() + 1, str_path.size());
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
}

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
}

void Editor3D::show()
{
	//Support variable
	bool menu_change = start_menu || Input::wasKeyPressed(SDL_SCANCODE_1) || Input::wasKeyPressed(SDL_SCANCODE_2) || Input::wasKeyPressed(SDL_SCANCODE_3) || Input::wasKeyPressed(SDL_SCANCODE_TAB) || Input::wasKeyPressed(SDL_SCANCODE_ESCAPE);

	//Select Menu Option
	if (current_layer == EditorLayer::LAYER1 && Input::wasKeyPressed(SDL_SCANCODE_1))
		menu_option = MenuOption::ADD, current_layer = EditorLayer::LAYER2;
	if (current_layer == EditorLayer::LAYER1 && Input::wasKeyPressed(SDL_SCANCODE_2))
		menu_option = MenuOption::EDIT, current_layer = EditorLayer::LAYER2;
	if (current_layer == EditorLayer::LAYER1 && Input::wasKeyPressed(SDL_SCANCODE_3))
		menu_option = MenuOption::REMOVE, current_layer = EditorLayer::LAYER2;

	//Select Entity Option
	if (current_layer == EditorLayer::LAYER2 && Input::wasKeyPressed(SDL_SCANCODE_1))
		entity_option = EntityOption::OBJECT, current_layer = EditorLayer::LAYER3;
	if (current_layer == EditorLayer::LAYER2 && Input::wasKeyPressed(SDL_SCANCODE_2))
		entity_option = EntityOption::LIGHT, current_layer = EditorLayer::LAYER3;
	if(current_layer == EditorLayer::LAYER2 && Input::wasKeyPressed(SDL_SCANCODE_3))
		entity_option = EntityOption::SOUND, current_layer = EditorLayer::LAYER3;

	//Selector
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
			else if (menu_option == MenuOption::EDIT || menu_option == MenuOption::ADD)
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
			else if (menu_option == MenuOption::EDIT || menu_option == MenuOption::ADD)
			{
				int next_sound = current_sound + 1;
				if (next_sound < scene->sounds.size()) current_sound = next_sound;
				else current_sound = 0;
			}

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
			entity_option == EntityOption::NO_ENTITY;
			current_layer = EditorLayer::LAYER2;
		}

	}

	//Menu
	if (menu_change)
	{
		switch (menu_option)
		{
		case(MenuOption::ADD):
			switch (entity_option) 
			{
			case(EntityOption::OBJECT):
				cout << "Select an asset type from the list (Click in the screen to add it)" << endl << endl;
				for (int i = 0; i < assets_size; ++i)
				{
					if (i == current_asset)
						cout << assets[i] << " <--" << endl;
					else
						cout << assets[i] << endl;
				}
				break;
			case(EntityOption::LIGHT):
				cout << "Select a light type from the list (Click in the screen to add it)" << endl << endl;
				for (int i = 0; i < lights_size; ++i)
				{
					if (i == current_light)
						cout << lights[i] << " <--" << endl;
					else
						cout << lights[i] << endl;
				}
				break;
			case(EntityOption::SOUND):
				cout << "Select a sound type from the list (Click in the screen to add it)" << endl << endl;
				for (int i = 0; i < sounds_size; ++i)
				{
					if (i == current_sound)
						cout << sounds[i] << " <--" << endl;
					else
						cout << sounds[i] << endl;
				}
				break;
			case(EntityOption::NO_ENTITY):
				cout << "Select an entity type" << endl << endl;
				cout << "1. Object" << endl;
				cout << "2. Light" << endl;
				cout << "3. Sound" << endl;
				break;
			}
			break;
		case(MenuOption::EDIT):
			switch (entity_option)
			{
			case(EntityOption::OBJECT):
				cout << "Click an object to start editing it" << endl;
				break;
			case(EntityOption::LIGHT):
				cout << "Select a light from the list (Press SPACE to edit)" << endl << endl;
				for (int i = 0; i < scene->lights.size(); ++i)
				{
					if (i == current_light)
						cout << scene->lights[i] << " <--" << endl;
					else
						cout << scene->lights[i] << endl;
				}
				break;
			case(EntityOption::SOUND):
				cout << "Select a sound from the list (Press SPACE to edit)" << endl << endl;
				for (int i = 0; i < scene->sounds.size(); ++i)
				{
					if (i == current_sound)
						cout << scene->sounds[i] << " <--" << endl;
					else
						cout << scene->sounds[i] << endl;
				}
				break;
			case(EntityOption::NO_ENTITY):
				cout << "Select an entity type" << endl << endl;
				cout << "1. Object" << endl;
				cout << "2. Light" << endl;
				cout << "3. Sound" << endl;
				break;
			}
			break;
		case(MenuOption::REMOVE):
			switch (entity_option)
			{
			case(EntityOption::OBJECT):
				cout << "Click an object to remove it" << endl;
				break;
			case(EntityOption::LIGHT):
				cout << "Select a light from the list (Press SPACE to remove)" << endl << endl;
				for (int i = 0; i < scene->lights.size(); ++i)
				{
					if (i == current_light)
						cout << scene->lights[i] << " <--" << endl;
					else
						cout << scene->lights[i] << endl;
				}
				break;
			case(EntityOption::SOUND):
				cout << "Select a sound from the list (Press SPACE to remove)" << endl << endl;
				for (int i = 0; i < scene->sounds.size(); ++i)
				{
					if (i == current_sound)
						cout << scene->sounds[i] << " <--" << endl;
					else
						cout << scene->sounds[i] << endl;
				}
				break;
			case(EntityOption::NO_ENTITY):
				cout << "Select an entity type" << endl << endl;
				cout << "1. Object" << endl;
				cout << "2. Light" << endl;
				cout << "3. Sound" << endl;
				break;
			}
			break;
		case(MenuOption::NO_MENU):
			cout << "Welcome to the editor mode" << endl << endl;
			cout << "1. Add entity" << endl;
			cout << "2. Edit entity" << endl;
			cout << "3. Remove entity" << endl;
			start_menu = false;
			break;
		}
	}

}

void Editor3D::work()
{
	if (menu_option == MenuOption::ADD && Input::wasKeyPressed(SDL_SCANCODE_SPACE))
	{
		addEntity();
	}
	else if (menu_option == MenuOption::EDIT)
	{
		if (entity_option == EntityOption::OBJECT && Input::isMousePressed(SDL_BUTTON(1)))
		{
			//Pick an entity with a ray cast
			//Send that entity to editEntity method
		}
		else if (entity_option == EntityOption::LIGHT && Input::wasKeyPressed(SDL_SCANCODE_SPACE))
		{
			editEntity(scene->lights[current_light]);
		}
		else if (entity_option == EntityOption::SOUND && Input::wasKeyPressed(SDL_SCANCODE_SPACE))
		{
			editEntity(scene->sounds[current_sound]);
		}
	}
	else if (menu_option == MenuOption::REMOVE)
	{
		if(entity_option == EntityOption::OBJECT && Input::isMousePressed(SDL_BUTTON(1)))
		{
			//Pick an entity with a ray cast
			//Send that entity to RemoveEntity method
		}
		else if(entity_option == EntityOption::LIGHT && Input::wasKeyPressed(SDL_SCANCODE_SPACE))
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

void Editor3D::addEntity() 
{

	if (entity_option == EntityOption::OBJECT) {
		//New object
		ObjectEntity* new_object = new ObjectEntity();

		//Asset paths
		string asset_name = assets[current_asset];
		string asset_path = "\data\assets\\" + asset_name;
		string asset_mesh = asset_path + "\\" + asset_name + ".obj";
		string asset_color = asset_path + "\color_texture.tga";
		string asset_normal = asset_path + "\normal_texture.tga";
		string asset_opacity = asset_path + "\opacity_texture.tga";
		string asset_metallic_roughness = asset_path + "\metallic_roughness_texture.tga";
		string asset_occlusion = asset_path + "\occlusion_texture.tga";
		string asset_emissive = asset_path + "\emissive_texture.tga";

		//Assets features
		new_object->mesh = Mesh::Get(asset_mesh.c_str());
		new_object->material->color_texture.texture = Texture::Get(asset_color.c_str());
		new_object->material->normal_texture.texture = Texture::Get(asset_normal.c_str());
		new_object->material->opacity_texture.texture = Texture::Get(asset_opacity.c_str());
		new_object->material->metallic_roughness_texture.texture = Texture::Get(asset_metallic_roughness.c_str());
		new_object->material->occlusion_texture.texture = Texture::Get(asset_occlusion.c_str());
		new_object->material->emissive_texture.texture = Texture::Get(asset_emissive.c_str());

		//Parse mtl
		//TODO

		//Position
		//Cast a ray get the object positon and set it with a translate

		//Add the new object to the scene
		scene->addEntity(new_object);

		//Feedback
		cout << "Object succesfully created" << endl;
	}
	else if (entity_option == EntityOption::LIGHT)
	{
		//New object
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

		//Position
		//Cast a ray get the object positon and set it with a translate

		//Add the new object to the scene
		scene->addEntity(new_light);

		//Feedback
		cout << "Light succesfully created" << endl;
	}
	else if (entity_option == EntityOption::SOUND)
	{
		//New object
		SoundEntity* new_sound = new SoundEntity();

		//Sound filename
		string sound_name = sounds[current_sound];
		new_sound->filename = "\data\assets\\" + sound_name;

		//Position
		//Cast a ray get the object positon and set it with a translate

		//Add the new object to the scene
		scene->addEntity(new_sound);

		//Feedback
		cout << "Sound succesfully created" << endl;
	}
}

void Editor3D::editEntity(Entity* entity)
{
}

void Editor3D::removeEntity(Entity* entity)
{
	scene->removeEntity(entity);
}
