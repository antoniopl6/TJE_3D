
#ifndef RENDERER_H
#define RENDERER_H

#pragma once
#include "scene.h"
#include "fbo.h"
#include <algorithm>


struct RenderCall {
	Mesh* mesh;
	Material* material;
	Matrix44 model;
	BoundingBox* world_bounding_box;
	float distance_to_camera;

	RenderCall() { distance_to_camera = 10.0f;}
	RenderCall(Mesh* mesh, Material* material, Matrix44 model, BoundingBox* world_bounding_box, Camera* camera) 
	{
			this->mesh = mesh;
			this->material = material;
			this->model = model;
			this->world_bounding_box = world_bounding_box;
			this->distance_to_camera = world_bounding_box->center.distance(camera->center);
	}
};

// This class is in charge of rendering anything in our system.
// Separating the render from anything else makes the code cleaner
class Renderer
{

public:

	//Application variables
	Scene* scene;
	Camera* camera;
	Shader* shaderGUI;

	//Render variables
	std::vector<RenderCall*> render_calls; // Here we store each RenderCall to be sent to the GPU.

	//GUIs
	Texture* collectItem;
	Texture* points[2];
	Texture* battery;
	Texture* dmgScreen;
	Texture* apple;
	Texture* key;
	Texture* enter;

	//Scenes
	Texture* title;
	Texture* introScene;
	Texture* tutorialScene;
	Texture* loadScene;
	Texture* diedScene;
	Texture* finalScene;
	Texture* exitX;
	Texture* restartX;
	Texture* diedTitle;
	Texture* continueX;

	//Tutorial textures
	Texture* keyboard;
	Texture* keyboard_fe;
	Texture* mouseTutorial;
	Texture* note;

	//Sphere
	Mesh* sphere_mesh = Mesh::Get("data/assets/sphere/sphere.obj");
	Texture* sphere_texture = Texture::Get("data/assets/sphere/white_wire_texture.png");
	Shader* sphere_shader = Shader::Get("data/shaders/pixel.vs", "data/shaders/texture.fs");

	//Constructor
	Renderer(Scene* scene, Camera* camera);

	//Renders several elements of the scene
	void renderScene(Scene* scene, Camera* camera);

	//Renders an image
	void renderImage(Texture* Image, int w, int h, int x, int y, Vector4 tex_range = Vector4(1, 1, 1, 1), Vector4 color = Vector4(1, 1, 1, 1), bool flipuv = true);

	//Renders the bounding a sound sphere
	void renderSoundSphere(SoundEntity* sound);

	//loads GUIs textures
	void loadGUIs();

	//render GUIs
	void renderGUIs();

	//Intialize the render calls vector
	void createRenderCalls();

	//Set scene uniforms
	void setSceneUniforms(Shader* shader);

	//Render a draw call
	void renderDrawCall(Shader* shader, RenderCall* rc, Camera* camera);

	//Render a basic draw call
	void renderDepthMap(RenderCall* rc, Camera* light_camera);

	//Singlepass lighting
	void SinglePassLoop(Shader* shader, Mesh* mesh);

	//Multipass lighting
	void MultiPassLoop(Shader* shader, Mesh* mesh);

	//Shadow Atlas
	void createShadowAtlas();
	void computeShadowMap();
	void showShadowAtlas();

};

#endif

