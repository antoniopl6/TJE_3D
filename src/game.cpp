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

Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;


/////////////////////////
//EntityMesh* emeshPrueba;

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

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000000.f); //set the projection, we want to be perspective

	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	/*mesh = Mesh::Get("data/GrimmFoxy.obj");
	
	texture = Texture::Get("data/GrimmFoxy_Shell.tga");*/
	mesh = Mesh::Get("data/island.ASE");

	texture = Texture::Get("data/island_color.tga");
	
	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	/////////////////////////////////////////////////////
	//emeshPrueba = new EntityMesh(Mesh::Get("data/GrimmFoxy.obj"), Texture::Get("data/GrimmFoxy_Shell.tga"), shader, Vector4(1, 1, 1, 1), Matrix44());
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

	///////////////////
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
void Game::RayPickCheck(Camera* cam) {
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrgin = cam->eye;

	Scene* scene = Scene::instance;
	std::vector<Entity*> entities = scene->objects;

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
   
	//create model matrix for cube
	//m.rotate(angle*DEG2RAD, Vector3(0, 1, 0));

	if(shader)
	{
		////Prueba para colocar la camara en tercera persona para el personaje
		//Vector3 eye = emeshPrueba->model * Vector3(0.0f, 300.0f, -50.0f);
		//Vector3 center = emeshPrueba->model * Vector3(0.0f, 0.0f, 400.0f);
		//Vector3 up = emeshPrueba->model.rotateVector(Vector3(0.0f, 1.0f, 0.0f));
		//camera->lookAt(eye, center, up);
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
	float speed = seconds_elapsed * mouse_speed * 5; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	angle += (float)seconds_elapsed * 10.0f;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_W)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D)) camera->move(Vector3(-1.0f,0.0f, 0.0f) * speed);

	//emeshPrueba->update(seconds_elapsed); //Actualiza la posicion de la prueba de personaje, con las flechas

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


			targetx = clamp(spawnPos.x /tileSizeX, 0, W);
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
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
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

