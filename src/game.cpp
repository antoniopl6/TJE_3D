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
float mouse_speed = 100.0f;
FBO* fbo = NULL;

using namespace std;

Game* Game::instance = NULL;
bool scene_saved = false;

///////////////prueba route
Route* route;
bool MonsterIsInPathRoute = false;
Point* closestPoint;
int idx = 0;
int numPoints;
double time2 = 0.0;
///////////////


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

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	//Create the scene
	scene = new Scene();

	//Create the main camera
	camera = new Camera();
	scene->main_camera = camera;

	//Load the scene JSON
	if (!scene->load("data/scene.json"))
		exit(1);

	//Create an entity editor
	entity_editor = new Editor3D(scene);

	//This class will be the one in charge of rendering the scene
	renderer = new Renderer(scene);

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse


	//////////////////////////prueba route
	std::vector<Vector3> points;
	Vector3* p0 = new Vector3(0, 0, 0);
	Vector3* p1 = new Vector3(1600, 0, 0);
	Vector3* p2 = new Vector3(1600, 0, 1600);
	Vector3* p3 = new Vector3(0, 0, 1600);
	points.push_back(*p0);
	points.push_back(*p1);
	points.push_back(*p2);
	points.push_back(*p3);
	numPoints = 2;
	route = new Route(100, 100, points);

	/////////////////////////
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

	//Enable view camera
	camera->enable();

	//Draw the floor grid
	//drawGrid();

	//Render the scene
	renderer->renderScene();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	//Update Main Character
	MainCharacterEntity* character = scene->main_character;
	if (character->bounding_box_trigger)
	{
		character->updateBoundingBox();
		character->bounding_box_trigger = false;
	}

	//Update Monster
	MonsterEntity* monster = scene->monster;
	if (monster->bounding_box_trigger)
	{
		monster->updateBoundingBox();
		monster->bounding_box_trigger = false;
	}
	//std::cout << "pos: x:" << monster->model.getTranslation().x << ", y: " << monster->model.getTranslation().z << std::endl;
	/*Vector2 start = route->getGridVector(monster->model.getTranslation().x, monster->model.getTranslation().y, monster->model.getTranslation().z);
	std::cout << "pos: x:" << start.x << ", y: " << start.y << std::endl;*/
	/*if (monster->isInFollowRange(camera)) {
		monster->updateFollow(elapsed_time, camera);
		MonsterIsInPathRoute = false;
	}*/
	//////////////////////////// path
	//else {
		Matrix44 model = monster->model;
		if (!MonsterIsInPathRoute) { //If monster do not have a route to follow generate one on path closestPoint
			closestPoint = route->getClosestPoint(model.getTranslation());
			Vector2 start = route->getGridVector(model.getTranslation().x, model.getTranslation().y, model.getTranslation().z);
			Point currentPoint = Point(start.x, start.y);
			closestPoint->SetPath(route->grid, currentPoint.startx, currentPoint.starty, route->W, route->H);
			MonsterIsInPathRoute = true;
			
		}
		else {
			Vector2 newPos = closestPoint->path[idx];
			Vector3 newTranslate = route->getSceneVector(newPos.x, newPos.y);
			if (monster->moveToTarget(elapsed_time, newTranslate))
				idx++;
				//std::cout << "Next target" << std::endl,
			if (idx == closestPoint->path_steps) {
				MonsterIsInPathRoute = false;
				idx = 0;
			}
		}
	//}
	

	//}
	///////////////////////////
	//Update Objects
	for (int i = 0; i < scene->objects.size(); ++i)
	{
		ObjectEntity* object = scene->objects[i];
		if (object->bounding_box_trigger) {
			object->updateBoundingBox();
			object->bounding_box_trigger = false;
		}
	}

	//Update Lights
	for (int i = 0; i < scene->lights.size(); i++)
	{
		//TODO
	}

	//Update Sounds
	for (int i = 0; i < scene->sounds.size(); i++)
	{
		//TODO
	}

	//Update main character camera
	scene->main_character->updateMainCamera(seconds_elapsed, mouse_speed, mouse_locked);

	//Render entity editor
	if (render_editor)
		entity_editor->render();

	//Save scene
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL) && Input::isKeyPressed(SDL_SCANCODE_S))
	{
		if (!scene_saved)
		{
			scene->save();
			scene_saved = true;
		}
	}

	if (Input::wasKeyPressed(SDL_BUTTON_LEFT))
		cout << "Here" << endl;
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
			camera->center *= -1.f;
			turn_around = true;
		}
		break;

		//Entity editor
	case SDLK_h:
		render_editor = !render_editor;
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
		case SDLK_q: camera->center *= -1.f;
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
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

