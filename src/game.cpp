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


/////////////////////////
EntityMesh* emeshPrueba;

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


	//load meshes and textures
	mesh = Mesh::Get("data/island.ASE");
	texture = Texture::Get("data/island_color.tga");
	
	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	/////////////////////////////////////////////////////
	emeshPrueba = new EntityMesh(Mesh::Get("data/GrimmFoxy.obj"), Texture::Get("data/GrimmFoxy_Shell.tga"), shader, Vector4(1, 1, 1, 1), Matrix44());
	
	//create our camera
	camera = new Camera();
	camera->lookAt(emeshPrueba->getPosition() + Vector3(0.f, 230.f, 300.f), emeshPrueba->getPosition() + Vector3(0.f, 230.f, 400.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000000.f); //set the projection, we want to be perspective

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
		emeshPrueba->render();
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
	float speed = seconds_elapsed * mouse_speed * 4; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	angle += (float)seconds_elapsed * 10.0f;

	//Boost speed
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 1.5; //move faster with left shift

	//mouse input to rotate the camera
	if (((Input::mouse_state) || mouse_locked) && !turn_around) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
	}

	//Camera front and side vectors
	Vector3 camera_front = Vector3();
	Vector3 camera_side = Vector3();
	if ((Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_D)) && !turn_around)
	{
		camera_front = ((camera->center - camera->eye) * Vector3(1.f, 0.f, 1.f)).normalize();
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_D))
		{
			camera_side = Vector3(-camera_front.z, 0.f, camera_front.x);
		}
			
	}

	//Move the camera along with the main character
	if (Input::isKeyPressed(SDL_SCANCODE_W) && !turn_around) camera->lookAt(camera->eye + camera_front * speed, camera->center + camera_front * speed, camera->up);
	if (Input::isKeyPressed(SDL_SCANCODE_A) && !turn_around) camera->lookAt(camera->eye - camera_side * speed, camera->center - camera_side * speed, camera->up);
	if (Input::isKeyPressed(SDL_SCANCODE_S) && !turn_around) camera->lookAt(camera->eye - camera_front * speed, camera->center - camera_front * speed, camera->up);
	if (Input::isKeyPressed(SDL_SCANCODE_D) && !turn_around) camera->lookAt(camera->eye + camera_side * speed, camera->center + camera_side * speed, camera->up);


	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
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
		case SDLK_q: camera->center *= -1.f, turn_around = false;
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

