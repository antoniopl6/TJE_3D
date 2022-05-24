
#ifndef RENDERER_H
#define RENDERER_H

#pragma once
#include "scene.h"
#include "fbo.h"
#include <algorithm>


struct RenderCall {
	Mesh* mesh;
	Material* material;
	Matrix44* model;
	BoundingBox* world_bounding_box;
	float distance_to_camera;

	RenderCall() { distance_to_camera = 10.0f;}
	RenderCall(Mesh* mesh, Material* material, Matrix44* model, BoundingBox* world_bounding_box, Camera* camera) 
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

	//Render variables
	std::vector<RenderCall*> render_calls; // Here we store each RenderCall to be sent to the GPU.

	//Constructor
	Renderer(Scene* scene);

	//Renders several elements of the scene
	void renderScene();

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

