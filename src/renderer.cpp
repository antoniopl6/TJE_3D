#include "renderer.h"
#include "game.h"

constexpr int SHOW_ATLAS_RESOLUTION = 300;
constexpr int SHADOW_MAP_RESOLUTION = 2048;

using namespace std;

//Constructor
Renderer::Renderer(Scene* scene)
{
	//Set scene and camera
	this->scene = scene;
	this->camera = scene->main_camera;

	//Create render calls vector
	createRenderCalls();

	//Create Shadow Atlas: We create a dynamic atlas to be resizable
	createShadowAtlas();
}

//Sort render calls by transparency and distance to camera
bool sortRenderCall(const RenderCall* rc1, const RenderCall* rc2)
{
	AlphaMode rc1_alpha = rc1->material->alpha_mode;
	AlphaMode rc2_alpha = rc2->material->alpha_mode;
	if (rc1_alpha == AlphaMode::BLEND && rc2_alpha != AlphaMode::BLEND) return false;
	else if (rc1_alpha != AlphaMode::BLEND && rc2_alpha == AlphaMode::BLEND) return true;
	else if (rc1_alpha == AlphaMode::BLEND && rc2_alpha == AlphaMode::BLEND) return rc1->distance_to_camera > rc2->distance_to_camera;
	else if (rc1_alpha != AlphaMode::BLEND && rc2_alpha != AlphaMode::BLEND) return rc1->distance_to_camera < rc2->distance_to_camera;
	else return true;
}

//Intialize render calls vector
void Renderer::createRenderCalls()
{
	//Clear the render calls vector
	render_calls.clear();

	//Main character render call
	MainCharacterEntity* mc = scene->main_character; 
	if (mc->visible && mc->mesh && mc->material)
		render_calls.push_back(new RenderCall(mc->mesh, mc->material, &mc->model, &mc->world_bounding_box, camera));

	//Monster render call
	MonsterEntity* monster = scene->monster;
	if (monster->visible && monster->mesh && monster->material)
		render_calls.push_back(new RenderCall(monster->mesh, monster->material, &monster->model, &monster->world_bounding_box, camera));

	//Objects render calls	
	for (int i = 0; i < scene->objects.size(); ++i)
	{
		ObjectEntity* object = scene->objects[i];
		if (object->visible && object->mesh && object->material)
		{
			render_calls.push_back(new RenderCall(object->mesh, object->material, &object->model, &object->world_bounding_box,camera));
		}
	}

	//Now we sort the RenderCalls vector according to the boolean method sortRenderCall
	sort(render_calls.begin(), render_calls.end(), sortRenderCall);
}

//Set the render shader and scene uniforms
void Renderer::setSceneUniforms(Shader* shader)
{
	//Upload scene uniforms
	shader->setUniform("u_ambient_light", scene->ambient_light);
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_time", getTime());
	shader->setUniform("u_num_shadows", (float)scene->num_shadows);

	//Shadow Atlas
	if (scene->shadow_atlas)
	{
		shader->setTexture("u_shadow_atlas", scene->shadow_atlas, 8);
		shader->setUniform("u_shadows", 1);
	}
	else
	{
		shader->setUniform("u_shadows", 0);
	}

}

void Renderer::renderScene()
{	
	//no shader? then nothing to render
	if (!scene->shader)
		return;

	//If there aren't lights in the scene don't render nothing
	if (scene->lights.empty()) 
		return;

	//Set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Check gl errors before starting
	checkGLErrors();

	//Compute Shadow Atlas (only spot light are able to cast shadows so far)
	computeShadowMap();

	//Enable the shader with which scene entities are rendered
	Shader* shader = scene->shader;
	shader->enable();

	//Set scene uniforms
	setSceneUniforms(shader);
	
	//Entity render
	for (int i = 0; i < render_calls.size(); i++)
	{
		RenderCall* rc = render_calls[i];
		renderDrawCall(shader, rc, camera);
	}

	//Disable the shader
	shader->disable();

	//Debug shadow maps
	if (scene->show_atlas) showShadowAtlas();

	//Reset scene triggers

}

//Render a draw call
void Renderer::renderDrawCall(Shader* shader, RenderCall* rc, Camera* camera)
{
	//In case there is nothing to do
	if (!rc->mesh || !rc->mesh->getNumVertices() || !rc->material)
		return;
	assert(glGetError() == GL_NO_ERROR);

	//Textures
	Texture* color_texture = NULL;
	Texture* emissive_texture = NULL;
	Texture* omr_texture = NULL;
	Texture* normal_texture = NULL;
	//Texture* occlusion_texture = NULL;

	//Texture loading
	color_texture = rc->material->color_texture.texture;
	emissive_texture = rc->material->emissive_texture.texture;
	omr_texture = rc->material->metallic_roughness_texture.texture;
	normal_texture = rc->material->normal_texture.texture;
	//occlusion_texture = rc->material->occlusion_texture.texture;

	//Texture check
	if (!color_texture)	color_texture = Texture::getWhiteTexture();
	if (!emissive_texture) emissive_texture = Texture::getBlackTexture();
	if (!omr_texture) omr_texture = Texture::getWhiteTexture();

	//Select the blending mode
	if (rc->material->alpha_mode == AlphaMode::BLEND)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else 
		glDisable(GL_BLEND);

	//Select whether to render both sides of the triangles
	if (rc->material->two_sided) 
		glDisable(GL_CULL_FACE);
	else 
		glEnable(GL_CULL_FACE);

	//Check gl errors
	assert(glGetError() == GL_NO_ERROR);

	//Upload textures
	if(color_texture) shader->setTexture("u_color_texture", color_texture, 0);
	if (emissive_texture) shader->setTexture("u_emissive_texture", emissive_texture, 1);
	if (omr_texture) shader->setTexture("u_omr_texture", omr_texture, 2);
	if (normal_texture) shader->setTexture("u_normal_texture", normal_texture, 3);
	//if(occlusion_texture) shader->setTexture("u_occlussion_texture", occlusion_texture, 4);

	//Normal mapping
	if (normal_texture) shader->setUniform("u_normal_mapping", 1);
	else shader->setUniform("u_normal_mapping", 0);

	//Upload entity uniforms
	shader->setUniform("u_model", rc->model);
	shader->setUniform("u_alpha_cutoff", rc->material->alpha_mode == AlphaMode::MASK ? rc->material->alpha_cutoff : 0); //this is used to say which is the alpha threshold to what we should not paint a pixel on the screen (to cut polygons according to texture alpha)

	//Single pass lighting
	SinglePassLoop(shader, rc->mesh);
}

//Singlepass lighting
void Renderer::SinglePassLoop(Shader* shader, Mesh* mesh)
{
	//Blending support
	glDepthFunc(GL_LEQUAL);

	//Loop variables
	int const lights_size = scene->lights.size();
	int const max_num_lights = 5; //Single pass lighting accepts at most 5 lights
	int starting_light = 0;
	int final_light = min(max_num_lights - 1, lights_size - 1);

	//Lights properties vectors
	std::vector<Vector3> lights_position;
	std::vector<Vector3> lights_color;
	std::vector<float> lights_intensity;
	std::vector<float> lights_max_distance;
	std::vector<int> lights_type;

	//Spot lights vectors
	std::vector<Vector3> spots_direction;
	std::vector<Vector2> spots_cone;

	//Directional lights vectors
	std::vector<Vector3> directionals_front;

	//Shadows vectors
	std::vector<int> cast_shadows;
	std::vector<float> shadows_index;
	std::vector<float> shadows_bias;
	std::vector<Matrix44> shadows_vp;

	//Reserve memory
	lights_position.reserve(max_num_lights);
	lights_color.reserve(max_num_lights);
	lights_intensity.reserve(max_num_lights);
	lights_max_distance.reserve(max_num_lights);
	lights_type.reserve(max_num_lights);
	spots_direction.reserve(max_num_lights);
	spots_cone.reserve(max_num_lights);
	directionals_front.reserve(max_num_lights);
	cast_shadows.reserve(max_num_lights);
	shadows_index.reserve(max_num_lights);
	shadows_bias.reserve(max_num_lights);
	shadows_vp.reserve(max_num_lights);

	//Single pass lighting
	while (starting_light < lights_size)
	{
		if (starting_light == 5)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			shader->setUniform("u_ambient_light", Vector3());
		}
		if (final_light == lights_size - 1) shader->setUniform("u_last_iteration", 1);
		else shader->setUniform("u_last_iteration", 0);

		//Support variables
		int num_lights = final_light - starting_light + 1;
		int j = 0;

		for (int i = starting_light; i <= final_light; i++)
		{
			//Current Light
			LightEntity* light = scene->lights[i];

			//General light properties
			lights_position[j] = light->model.getTranslation();
			lights_color[j] = light->color;
			lights_intensity[j] = light->intensity;
			lights_max_distance[j] = light->max_distance;

			//Specific light properties
			switch (light->light_type)
			{
				case(LightType::POINT_LIGHT):
					lights_type[j] = light->light_type;
					break;
				case (LightType::SPOT_LIGHT):
					spots_direction[j] = light->model.rotateVector(Vector3(0, 0, -1));
					spots_cone[j] = Vector2(light->cone_exp, cos(light->cone_angle * DEG2RAD));
					lights_type[j] = light->light_type;
					break;
				case (LightType::DIRECTIONAL_LIGHT):
					directionals_front[j] = light->model.rotateVector(Vector3(0, 0, -1));
					lights_type[j] = light->light_type;
					break;
			}

			//Shadow properties
			if (scene->shadow_atlas && light->cast_shadows)
			{		
				cast_shadows[j] = 1;
				shadows_index[j] = (float) light->shadow_index;
				shadows_bias[j] = light->shadow_bias;
				shadows_vp[j] = light->shadow_camera->viewprojection_matrix;	
			}
			else
			{
				cast_shadows[j] = 0;
			}

			//Update iterator
			j++;		
		}

		//Upload light uniforms
		shader->setUniform3Array("u_lights_position", (float*)&lights_position[0], num_lights);
		shader->setUniform3Array("u_lights_color", (float*)&lights_color[0], num_lights);
		shader->setUniform1Array("u_lights_intensity", &lights_intensity[0], num_lights);
		shader->setUniform1Array("u_lights_max_distance", &lights_max_distance[0], num_lights);
		shader->setUniform1Array("u_lights_type", &lights_type[0], num_lights);
		shader->setUniform3Array("u_spots_direction", (float*)&spots_direction[0], num_lights);
		shader->setUniform2Array("u_spots_cone", (float*)&spots_cone[0], num_lights);
		shader->setUniform3Array("u_directionals_front", (float*)&directionals_front[0], num_lights);
		shader->setUniform("u_num_lights", num_lights);

		//Upload shadow uniforms
		shader->setUniform1Array("u_cast_shadows", &cast_shadows[0], num_lights);
		shader->setUniform1Array("u_shadows_index", &shadows_index[0], num_lights);
		shader->setUniform1Array("u_shadows_bias", &shadows_bias[0], num_lights);
		shader->setMatrix44Array("u_shadows_vp", &shadows_vp[0], num_lights);

		//do the draw call that renders the mesh into the screen
		mesh->render(GL_TRIANGLES);

		//Update variables
		starting_light = final_light + 1;
		final_light = min(max_num_lights + final_light, lights_size - 1);

	}

	//set the render state as it was before to avoid problems with future renders
	glDisable(GL_BLEND);
	glDepthFunc(GL_LESS);
}

//Render basic draw call
void Renderer::renderDepthMap(RenderCall* rc, Camera* light_camera)
{
	//In case there is nothing to do
	if (!rc->mesh || !rc->mesh->getNumVertices() || !rc->material)
		return;
	assert(glGetError() == GL_NO_ERROR);

	//Define locals to simplify coding
	Shader* shader = NULL;

	//Select whether to render both sides of the triangles
	if (rc->material->two_sided) glDisable(GL_CULL_FACE);
	else glEnable(GL_CULL_FACE);
	assert(glGetError() == GL_NO_ERROR);

	//chose a shader
	shader = Shader::Get("data/shaders/depth.vs", "data/shaders/color.fs");
	assert(glGetError() == GL_NO_ERROR);

	//no shader? then nothing to render
	if (!shader)
		return;
	shader->enable();

	//Upload scene uniforms
	shader->setUniform("u_model", rc->model);
	shader->setUniform("u_viewprojection", light_camera->viewprojection_matrix);
	shader->setUniform("u_alpha_cutoff", rc->material->alpha_mode == AlphaMode::MASK ? rc->material->alpha_cutoff : 0); //this is used to say which is the alpha threshold to what we should not paint a pixel on the screen (to cut polygons according to texture alpha)

	//Disable blending
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);

	//do the draw call that renders the mesh into the screen
	rc->mesh->render(GL_TRIANGLES);

	//disable shader
	shader->disable();
}

//Create a shadow atlas
void Renderer::createShadowAtlas()
{
	//Compute the number of shadows of the scene and the shadow index of each light
	int shadow_index = 0;
	for (int i = 0; i < scene->lights.size(); i++)
	{
		LightEntity* light = scene->lights[i];
		if (light->cast_shadows)
		{
			//Assign a shadow slot for shadow atlas
			light->shadow_index = shadow_index;

			//Update
			scene->num_shadows++;
			shadow_index++;
		}
	}

	//Delete the former atlas and continue
	if (scene->fbo)
	{
		delete scene->fbo;
		scene->fbo = NULL;
		scene->shadow_atlas = NULL;

		if (scene->num_shadows == 0)
			return;

	}

	//New shadow atlas
	scene->fbo = new FBO();
	scene->fbo->setDepthOnly(SHADOW_MAP_RESOLUTION * scene->num_shadows, SHADOW_MAP_RESOLUTION); //We create it wide in order to save memory space and improve shadow map management
	scene->shadow_atlas = scene->fbo->depth_texture;
}

//Compute spot shadow maps into the shadow atlas
void Renderer::computeShadowMap()
{
	//Compute Shadow Atlas
	if (!scene->fbo || !scene->shadow_atlas)
		return;

	//Boost performance
	glColorMask(false, false, false, false);
	//Bind the fbo
	scene->fbo->bind();

	//Iterate over light vector
	for (int i = 0; i < scene->lights.size(); i++)
	{
		//Current light
		LightEntity* light = scene->lights[i];

		//For the first render
		if (!light->shadow_camera) light->shadow_camera = new Camera();

		//Set the atlas region of the shadow map to work on
		Vector4 shadow_region(light->shadow_index * SHADOW_MAP_RESOLUTION, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);

		//Activate flags on the shadow region
		glViewport(shadow_region.x, shadow_region.y, shadow_region.z, shadow_region.w);
		glScissor(shadow_region.x, shadow_region.y, shadow_region.z, shadow_region.w);
		glEnable(GL_SCISSOR_TEST);

		//Clear Depth Buffer on the shadow region
		glClear(GL_DEPTH_BUFFER_BIT);

		//Light camera
		Camera* shadow_camera = light->shadow_camera;

		//Camera properties
		float camera_fov = 2 * light->cone_angle;
		float camera_aspect = SHADOW_MAP_RESOLUTION / SHADOW_MAP_RESOLUTION;
		float camera_near = 0.1f;
		float camera_far = light->max_distance;
		Vector3 camera_position = light->model.getTranslation();
		Vector3 camera_front = light->model * Vector3(0, 0, -1);
		Vector3 camera_up = light->model.rotateVector(Vector3(0, 1, 0));

		//Set Perspective Matrix
		shadow_camera->setPerspective(camera_fov, camera_aspect, camera_near, camera_far);

		//Set View Matrix
		shadow_camera->lookAt(camera_position, camera_front, camera_up);

		//Enable camera
		shadow_camera->enable();

		for (int i = 0; i < render_calls.size(); ++i)
		{
			RenderCall* rc = render_calls[i];
			if (rc->material->alpha_mode == AlphaMode::BLEND)
				continue;
			if (shadow_camera->testBoxInFrustum(rc->world_bounding_box->center, rc->world_bounding_box->halfsize))
			{
				renderDepthMap(rc, shadow_camera);
			}
		}

	}

	//Unbind the fbo
	scene->fbo->unbind();

	//Reset
	Game* game = Game::instance;
	glViewport(0, 0, game->window_width, game->window_height);
	glColorMask(true, true, true, true);
	glDisable(GL_SCISSOR_TEST);

}

//Print shadow map in the screen
void Renderer::showShadowAtlas()
{
	//Just in case
	if (!scene->shadow_atlas || scene->num_shadows == 0)
		return;

	//Some variables
	Game* game = Game::instance;
	int num_shadows = scene->num_shadows;
	int window_height = game->window_height;
	int window_width = game->window_width;
	int num_shadows_per_scope = min(num_shadows, window_width / SHOW_ATLAS_RESOLUTION); //Number of shadow maps that will be displayed at the same time (within a scope).
	int shadow_scope = clamp(scene->atlas_scope, 0, ceil(num_shadows / (float)num_shadows_per_scope) - 1); //Current shadow scope.
	int num_shadows_in_scope = min(num_shadows - shadow_scope * num_shadows_per_scope, num_shadows_per_scope); //Number of shadows in the current scope
	int shadow_offset = (window_width - num_shadows_in_scope * SHOW_ATLAS_RESOLUTION) / 2; //In order to place the shadow maps in the center of the horizontal axis.
	int starting_shadow = shadow_scope * num_shadows_per_scope;
	int final_shadow = starting_shadow + num_shadows_in_scope;

	for (int i = 0; i < scene->lights.size(); ++i)
	{
		//Current light
		LightEntity* light = scene->lights[i];
		if (light->cast_shadows)
		{
			//Only render if lights are in the right scope
			if (starting_shadow <= light->shadow_index && light->shadow_index < final_shadow)
			{

				//Map shadow map into screen coordinates
				glViewport((light->shadow_index - starting_shadow) * SHOW_ATLAS_RESOLUTION + shadow_offset, 0, SHOW_ATLAS_RESOLUTION, SHOW_ATLAS_RESOLUTION);

				//Render the shadow map with the linearized shader
				Shader* shader = Shader::Get("quad.vs","linearize.fs");
				shader->enable();
				shader->setUniform("u_camera_nearfar", Vector2(light->shadow_camera->near_plane, light->shadow_camera->far_plane));
				shader->setUniform("u_shadow_index", (float)light->shadow_index);
				shader->setUniform("u_num_shadows", (float)num_shadows);
				scene->shadow_atlas->toViewport(shader);
				shader->disable();
			}
		}
	}

	//Update shadow scope
	scene->atlas_scope = shadow_scope;

	//Reset
	glViewport(0, 0, window_width, window_height);

}