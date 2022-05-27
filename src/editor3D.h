#pragma once
#include "game.h"

enum MenuOption {
	NONE,
	ADD,
	EDIT,
	REMOVE
};

enum MenuEntity
{
	NONE,
	OBJECT,
	LIGHT,
	SOUND
};

class editor3D
{
public:
	
	//Game variables
	Scene* scene;

	//Menu display
	MenuOption menu_option;
	MenuEntity menu_entity;

	//Constructor
	editor3D(Scene* scene);
	void renderEditor();

};
