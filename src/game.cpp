#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "entity.h"
#include "entitymesh.h"
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

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//Create the scene
	scene = new Scene();
	
	//Create the main camera
	camera = new Camera();
	scene->main_camera = camera;

	//Load the scene JSON
	if (!scene->load("data/scene.json"))
		exit(1);

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}
void Game::RenderTerrainExample() {
	Matrix44 m;
	Vector3 halfSize = mesh->box.halfsize * 2;
	EntityMesh* emeshIsland = new EntityMesh(mesh, texture, shader, Vector4(1, 1, 1, 1), m);
	float lodDistance = 200.0f;
	float noRenderDist = 1000000.0f;

	for (size_t i = 0; i < 10; i++)
	{
		for (size_t j = 0; j < 10; j++)
		{
			emeshIsland->model = m;
			m.setTranslation(halfSize.x * i, 0.0f, halfSize.z * j);

			
			Vector3 emeshPos = m.getTranslation();
			//No renderizar lo que no este en vision de camara
			BoundingBox worldAABB = transformBoundingBox(m, mesh->box);
			if (!camera->testBoxInFrustum(worldAABB.center, worldAABB.halfsize)) {
				continue;
			}
			Vector3 camPos = camera->eye;
			float dist = emeshPos.distance(camPos);

			if (dist > noRenderDist) {
				continue;
			}
			//Ejemplo para renderizar un elemento a mas baja calidad una vez esta lo suficientemente lejos y asi conseguir eficiencia
			if (dist < lodDistance) {
				//Render otra isla a mas baja calidad
			}

			emeshIsland->render();
		}
	}
}

/*
void Game::RayPickCheck(Camera* cam) {
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrgin = cam->eye;

	Scene scene = Scene::getInstance();
	std::vector<Entity*> entities = scene.entities;

	for (size_t i = 0; i < entities.size(); i++)
	{
		//downCast, se devería de hacer de una clase que fuera pickeable, cambiar a una clase diferente
		EntityMesh* entity = (EntityMesh*)entities[i];
		//si se puede hacer
		if (entity) {
			Vector3 pos;
			Vector3 normal;
			if (entity->mesh->testRayCollision(entity->model, rayOrgin, dir, pos, normal)) {
				//Se cogería el objeto
				scene.eraseEntity(i);
			}
		}
	}
}
*/

//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
  

	if(shader)
	{
		//Prueba para colocar la camara en tercera persona para el personaje
		//emeshPrueba->render();
		//RenderTerrainExample();
	}

	//Draw the floor grid
	drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	//Update main character camera
	scene->main_character->updateMainCamera(seconds_elapsed, mouse_speed, mouse_locked);
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
		
		//Turn around
		case SDLK_q: 
			if (!turn_around)
			{
				camera->center *= -1.f;
				turn_around = true;
			}
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

