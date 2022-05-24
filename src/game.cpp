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

	//This class will be the one in charge of rendering all 
	renderer = new Renderer();

	//Create Shadow Atlas: We create a dynamic atlas to be resizable
	renderer->createShadowAtlas(scene);

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

	//IA pathfinding initialization
	PruebaIAClase();
}

int W = 100;
int H = 100;
float tileSizeX = 10.0f;
float tileSizeY = 10.0f;
int startx;
int starty;
int targetx;
int targety;
uint8* grid;

void Game::PruebaIAClase() {
	// the map info should be an array W*H of bytes where 0 means block, 1 means walkable
	grid = new uint8[W*H];
	for (size_t i = 0; i < W*H; i++)
	{
		grid[i] = 1;
	}


};

/*
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
*/

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
	////set the clear color (the background color)
	//glClearColor(0.0, 0.0, 0.0, 1.0);

	//// Clear the window and the depth buffer
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	////set the camera as default
	//camera->enable();

	////set flags
	//glDisable(GL_BLEND);
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
 // 

	////if(shader)
	////{
	//	//Prueba para colocar la camara en tercera persona para el personaje
	//	//emeshPrueba->render();
	//	//RenderTerrainExample();
	////Vector3 eye = scene->main_character->model * Vector3(0.0f, 300.0f, -50.0f);
	////	Vector3 center = scene->main_character->model * Vector3(0.0f, 0.0f, 400.0f);
	////	Vector3 up = scene->main_character->model.rotateVector(Vector3(0.0f, 1.0f, 0.0f));
	////	camera->lookAt(eye, center, up);

	renderer->renderScene(scene);

	//scene->renderEntities();
	//}

	//Draw the floor grid
	drawGrid();
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

	//Update Objects
	for (int i = 0; i < scene->objects.size(); ++i)
	{
		ObjectEntity* object = scene->objects[i];
		if (object->bounding_box_trigger) {
			object->updateBoundingBox();
		}
	}

	//Update Lights

	//Update main character camera
	scene->main_character->updateMainCamera(seconds_elapsed, mouse_speed, mouse_locked);

	//Save scene
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL) && Input::isKeyPressed(SDL_SCANCODE_S))
	{
		if (!scene_saved)
		{
			scene->save();
			scene_saved = true;
		}
	}

}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
	case SDLK_F1: Shader::ReloadAll(); break;
	case SDLK_q:
		if (!turn_around)
		{
			camera->center *= -1.f;
			turn_around = true;
		}
		/////Ejemplo para probar IA y encontrar caminos, set starting points with 3 and then set target one with 4
	case SDLK_3: {
		Vector2 mouse = Input::mouse_position;
		Game* g = Game::instance;
		Vector3 dir = camera->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
		Vector3 rayOrigin = camera->eye;

		Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);
		startx = clamp(spawnPos.x / tileSizeX, 0, W);
		starty = clamp(spawnPos.z / tileSizeY, 0, H);
		break;
	}
	case SDLK_4: {
		Vector2 mouse = Input::mouse_position;
		Game* g = Game::instance;
		Vector3 dir = camera->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
		Vector3 rayOrigin = camera->eye;

		Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);


		targetx = clamp(spawnPos.x / tileSizeX, 0, W);
		targety = clamp(spawnPos.z / tileSizeY, 0, H);

		//here we must fill the map with all the info
//...
//when we want to find the shortest path, this array contains the shortest path, every value is the Nth position in the map, 100 steps max
		int output[100];

		//we call the path function, it returns the number of steps to reach target, otherwise 0
		int path_steps = AStarFindPathNoTieDiag(
			startx, starty, //origin (tienen que ser enteros)
			targetx, targety, //target (tienen que ser enteros)
			grid, //pointer to map data
			W, H, //map width and height
			output, //pointer where the final path will be stored
			100); //max supported steps of the final path

	//check if there was a path
		if (path_steps != -1)
		{
			for (int i = 0; i < path_steps; ++i)
				std::cout << "X: " << (output[i] % W) << ", Y: " << floor(output[i] / W) << std::endl;
		}
		break;
	}
	/*case SDLK_9: {
		printf("x: %f, y: %f, z: %f", camera->center.x, camera->center.y, camera->center.z);
	}*/
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

