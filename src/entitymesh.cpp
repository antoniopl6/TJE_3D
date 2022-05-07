#include "entitymesh.h"

EntityMesh::EntityMesh(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, Matrix44 model) : Entity::Entity(model) {
	assert(mesh != null, "Null mesh given");
	assert(texture != null, "Null texture given");
	assert(shader != null, "Null shader given");
	this->mesh = mesh;
	this->texture = texture;
	this->shader = shader;
	this->color = color;
}



void EntityMesh::render()
{
	//get the last camera that was activated
	Camera* camera = Camera::current;
	Matrix44 model = this->model;

	//enable shader and pass uniforms
	shader->enable();
	shader->setUniform("u_color", color);
	shader->setUniform("u_model", model);
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setTexture("u_texture", texture, 0);

	//render the mesh using the shader
	mesh->render(GL_TRIANGLES);

	//disable the shader after finishing rendering
	shader->disable();
}
void EntityMesh::update(float dt) {
	float speed = dt * 100; //the speed is defined by the seconds_elapsed so it goes constant
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift, para correr
	if (Input::isKeyPressed(SDL_SCANCODE_UP)) model.translate(0.0f, 0.0f, speed);
	if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) model.translate(0.0f, 0.0f, -speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) model.rotate(dt, Vector3(0.0f, -1.0f, 0.0f));
	if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) model.rotate(dt, Vector3(0.0f, 1.0f, 0.0f));
}