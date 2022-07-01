/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef GAME_H
#define GAME_H

#pragma once
#include "includes.h"
#include "camera.h"
#include "utils.h"
#include "scene.h"
#include "editor3D.h"
#include "renderer.h"
#include "stage.h"

class Game
{
public:
	static Game* instance;

	STAGE_ID current_stage;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
	float time;
	float elapsed_time;
	int fps;
	bool must_exit;
	bool render_editor;
	bool scene_saved = false;
	float mouse_speed = 100.0f;

	//some vars
	Camera* main_camera; //our global camera
	Scene* scene; //game scene
	Editor3D* entity_editor;
	Renderer* renderer; //game render class
	bool mouse_locked; //tells if the mouse is locked (not seen)

	Game(int window_width, int window_height, SDL_Window* window);

	void RenderTerrainExample();

	void RayPickCheck(Camera* cam);

	//main functions
	void render(void);
	void update(double dt);

	//events
	void onKeyDown(SDL_KeyboardEvent event);
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown(SDL_MouseButtonEvent event);
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);

};


#endif 