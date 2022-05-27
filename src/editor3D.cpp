#include "editor3D.h"

editor3D::editor3D(Scene* scene)
{
	this->scene = scene;
	menu_option = MenuOption::NONE;
	menu_entity = MenuEntity::NONE;
}

void editor3D::renderEditor()
{
	if (menu_option != MenuOption::ADD && Input::isKeyPressed(SDL_SCANCODE_1))
		menu_option = MenuOption::ADD;
	if (menu_option != MenuOption::EDIT && Input::isKeyPressed(SDL_SCANCODE_2))
		menu_option = MenuOption::EDIT;
	if (menu_option != MenuOption::REMOVE && Input::isKeyPressed(SDL_SCANCODE_3))
		menu_option = MenuOption::REMOVE;
}
