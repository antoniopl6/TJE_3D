#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "entity.h"
#include "scene.h"


#include <cmath>

//some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
bool turn_around = false;

Animation* anim = NULL;
float angle = 0;
FBO* fbo = NULL;

using namespace std;

Game* Game::instance = NULL;



Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_editor = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	current_stage = STAGE_ID::INTRO;

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	//Create the scene
	scene = new Scene();

	//Create the main camera
	main_camera = new Camera();
	scene->main_camera = main_camera;

	//Load the scene JSON
	if (!scene->load("data/scene.json"))
		exit(1);
	
	//Create an entity editor
	entity_editor = new Editor3D(scene);

	//This class will be the one in charge of rendering the scene
	renderer = new Renderer(scene, main_camera);

	curr_stage = STAGE_ID::INTRO;

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
	
}

//what to do when the image has to be draw
void Game::render(void)
{
	//Set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Check gl errors before starting
	checkGLErrors();

	switch (current_stage) {
	case(STAGE_ID::INTRO):
		IntroStage::render();
		break;
	case(STAGE_ID::TUTORIAL):
		TutorialStage::render();
		break;
	case(STAGE_ID::PLAY):
		PlayStage::render();
		break;
	case(STAGE_ID::DIED):
		DiedStage::render();
		break;
	}

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(window);
}

void Game::update(double seconds_elapsed)
{
	switch(current_stage) {
	case(STAGE_ID::INTRO):
		current_stage = IntroStage::update(seconds_elapsed);
		break;
	case(STAGE_ID::TUTORIAL):
		current_stage = TutorialStage::update(seconds_elapsed);
		break;
	case(STAGE_ID::PLAY):
		current_stage = PlayStage::update(seconds_elapsed);
		break;
	case(STAGE_ID::DIED):
		current_stage = DiedStage::update(seconds_elapsed);
		break;
	}

}


//Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE:
		if (!render_editor)
			must_exit = true; //ESC key, kill the app
		break;
	case SDLK_F1: Shader::ReloadAll(); break;
		//Turn around
	case SDLK_q:
		if (!turn_around)
		{
			main_camera->center *= -1.f;
			turn_around = true;
		}
		break;

		//Entity editor
	case SDLK_h:
		render_editor = !render_editor;
		//entity_editor->current_camera = Editor3D::MAIN;
		if (render_editor)
			entity_editor->reset();
		break;
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
		//Keep looking forward
		case SDLK_q: main_camera->center *= -1.f;
	}
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	main_camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

