#include "renderer.h"
#include "game.h"
#include "framework.h"
#include "extra/hdre.h"

constexpr int SHOW_ATLAS_RESOLUTION = 300;
constexpr int SHADOW_MAP_RESOLUTION = 2048;

using namespace std;

//Constructor
Renderer::Renderer(Scene* scene, Camera* camera)
{
	//Set scene and camera to NULL
	this->scene = scene;
	this->camera = camera;
	
	//Creates shader for GUI and loads all textures
	shaderGUI = Shader::Get("data/shaders/image.vs", "data/shaders/image.fs");
	loadGUIs();

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

//loads GUIs textures
void Renderer::loadGUIs() {
	collectItem = Texture::Get("data/GUIs/collect.png");
	battery = Texture::Get("data/GUIs/battery.png");
	points[0] = Texture::Get("data/GUIs/circle.png");
	points[1] = Texture::Get("data/GUIs/grey.png");
	dmgScreen = Texture::Get("data/GUIs/dmg.png");
	apple = Texture::Get("data/GUIs/shiny-apple.png");
	key = Texture::Get("data/GUIs/key.png");
	enter = Texture::Get("data/GUIs/enter.png");

	//Scenes
	introScene = Texture::Get("data/screens/Intro-wallpaper.jpg");
	title = Texture::Get("data/screens/title.png");
	tutorialScene = Texture::Get("data/screens/4165805.jpg");
	diedScene = Texture::Get("data/screens/died.jpg");
	finalScene = Texture::Get("data/screens/died.jpg");

	keyboard = Texture::Get("data/screens/tutorial_GUIs/5650714.png");
	keyboard_fe = Texture::Get("data/screens/tutorial_GUIs/FE.png");
	mouseTutorial = Texture::Get("data/screens/tutorial_GUIs/mouse.png");
	continueX = Texture::Get("data/GUIs/continue2.png");
	diedTitle = Texture::Get("data/GUIs/diedTitle.png");
	exitX = Texture::Get("data/GUIs/exit.png");
	restartX = Texture::Get("data/GUIs/restart.png");
	note = Texture::Get("data/screens/tutorial_GUIs/note.png");
}

void Renderer::renderGUIs() {

	Game* g = Game::instance; 
	int screenWidth = g->window_width;
	int screenHeight = g->window_height;

	//if a collectable is in range to pick notify the player
	if (scene->collectableInRange()) {
		renderImage(collectItem, 300, 40, screenWidth / 2, screenHeight / 2 + 150, Vector4(0, 0, 1, 1));
		renderImage(points[1], 6, 6, screenWidth / 2, screenHeight / 2, Vector4(0, 0, 1, 1));
	}
	if (scene->hasDoorInRange() && g->scene->main_character->num_keys >= 1) {
		renderImage(enter, 300, 40, screenWidth / 2, screenHeight / 2 + 150, Vector4(0, 0, 1, 1));
		renderImage(points[1], 6, 6, screenWidth / 2, screenHeight / 2, Vector4(0, 0, 1, 1));
	}
	else {
		renderImage(points[0], 6, 6, screenWidth / 2, screenHeight / 2, Vector4(0, 0, 1, 1), Vector4(0.7, 0.7, 0.7, 0.7));
	}

	//Renders battery GUIs indicators
	float batteryPerc = scene->main_character->battery;
	if (0.0f <= batteryPerc && batteryPerc < 25.0f) {
		renderImage(battery, 50, 35, 40, 35, Vector4(0, 0, 1, 0.25), Vector4(0.8, 0.8, 0.8, 0.8));
	}
	else if (25.0f <= batteryPerc && batteryPerc < 50.0f) {
		renderImage(battery, 50, 35, 40, 35, Vector4(0, 0.25, 1, 0.25), Vector4(0.8, 0.8, 0.8, 0.8));
	}
	else if (50.0f <= batteryPerc && batteryPerc < 75.0f) {
		renderImage(battery, 50, 35, 40, 35, Vector4(0, 0.5, 1, 0.25), Vector4(0.8, 0.8, 0.8, 0.8));
	}
	else {
		renderImage(battery, 50, 35, 40, 35, Vector4(0, 0.75, 1, 0.25), Vector4(0.8, 0.8, 0.8, 0.8));
	}

	//Renders health GUIs indicators
	float health = scene->main_character->health;
	if (0.0f <= health && health <= 25.0f) {
		renderImage(dmgScreen, screenWidth, screenHeight, screenWidth / 2, screenHeight / 2, Vector4(0, 0, 1, 1), Vector4(0.8, 0.8, 0.8, 0.8));
	}
	else if (25.0f < health && health <= 50.0f) {
		renderImage(dmgScreen, screenWidth, screenHeight, screenWidth / 2, screenHeight / 2, Vector4(0, 0, 1, 1), Vector4(0.6, 0.6, 0.6, 0.6));
	}
	else if (50.0f < health && health <= 75.0f) {
		renderImage(dmgScreen, screenWidth, screenHeight, screenWidth / 2, screenHeight / 2, Vector4(0, 0, 1, 1), Vector4(0.3, 0.3, 0.3, 0.3));
	}

	//Render GUI apple and key
	int num_apples = scene->main_character->num_apples;
	for (size_t i = 0; i < num_apples; i++)
	{
		renderImage(apple, 35, 35, 110 + i * 8, 35, Vector4(0, 0, 1, 1));
	}
	int num_keys = scene->main_character->num_keys;
	for (size_t i = 0; i < num_keys; i++)
	{
		renderImage(key, 35, 35, 190 + i * 8, 35, Vector4(0, 0, 1, 1));
	}
}

//Intialize render calls vector
void Renderer::createRenderCalls()
{
	//Clear the render calls vector
	render_calls.clear();

	//Main character render call
	MainCharacterEntity* mc = scene->main_character;
	if (mc->visible && mc->mesh && mc->material)
		render_calls.push_back(new RenderCall(mc->mesh, mc->material, mc->model, &mc->world_bounding_box, camera));

	//Monster render call
	MonsterEntity* monster = scene->monster;
	if (monster->visible && monster->mesh && monster->material)
		render_calls.push_back(new RenderCall(monster->mesh, monster->material, monster->model, &monster->world_bounding_box, camera));

	//Objects render calls	
	for (int i = 0; i < scene->objects.size(); ++i)
	{
		ObjectEntity* object = scene->objects[i];
		if (object->visible && object->mesh && object->material)
		{
			Matrix44 global_matrix = object->computeGlobalModel();
			render_calls.push_back(new RenderCall(object->mesh, object->material, global_matrix, &object->world_bounding_box, camera));
		}
			
			
	}

	//Now we sort the RenderCalls vector according to the boolean method sortRenderCall
	sort(render_calls.begin(), render_calls.end(), sortRenderCall);
}

//Renders several elements of the scene
void Renderer::renderScene(Scene* scene, Camera* camera)
{
	//Set the scene and the camera with which to render
	this->scene = scene;
	this->camera = camera;

	//Create render calls vector
	createRenderCalls();

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

	//Use global model for flashlight
	Matrix44 local_model = scene->main_character->light->model;
	scene->main_character->light->model = local_model * scene->main_character->model;

	//Enable shader
	scene->shader->enable();

	//Entity render
	setSceneUniforms(scene->shader);
	for (int i = 0; i < render_calls.size(); i++)
	{
		RenderCall* rc = render_calls[i];
		if (camera->testBoxInFrustum(rc->world_bounding_box->center, rc->world_bounding_box->halfsize))
			renderDrawCall(scene->shader, rc, camera);
	}

	//Disable shader
	scene->shader->disable();

	//Reset flashlight local model
	scene->main_character->light->model = local_model;

	//Debug shadow maps
	if (scene->show_atlas) showShadowAtlas();

}

//Renders an image
void Renderer::renderImage(Texture* Image, int w, int h, int x, int y, Vector4 tex_range, Vector4 color, bool flipuv)
{
	//Check if there is an image
	if (!Image)
		return;

	//Disable and enable OpenGL flags
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Local variables
	Mesh quad;
	Camera cam2d;

	//Intialize local variables
	quad.createQuad(x, y, w, h, flipuv);
	cam2d.setOrthographic(0, Game::instance->window_width, Game::instance->window_height, 0, -1, 1);

	//Check if the shader exists
	if (!shaderGUI)
		return;

	//Set shader uniforms
	shaderGUI->enable();
	shaderGUI->setUniform("u_color", color);
	shaderGUI->setUniform("u_viewprojection", cam2d.viewprojection_matrix);
	shaderGUI->setUniform("u_texture", Image, 0);
	shaderGUI->setUniform("u_time", time);
	shaderGUI->setUniform("u_tex_range", tex_range);
	shaderGUI->setUniform("u_model", Matrix44());

	//Render the quad	
	quad.render(GL_TRIANGLES);

	//Disable the shader
	shaderGUI->disable();

	//Reset OpenGL flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

}

//Renders the bounding a sound sphere
void Renderer::renderSoundSphere(SoundEntity* sound)
{
	//Sphere mesh check
	if (!sphere_mesh || !sphere_shader || !sphere_texture)
		return;

	//Sound model
	Vector3 sound_position = sound->getPosition();
	float sound_area = sound->radius;

	//Sphere model
	Matrix44 sphere_model;
	sphere_model.translate(sound_position.x, sound_position.y, sound_position.z);
	sphere_model.scale(sound_area, sound_area, sound_area);

	//Set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable shader
	sphere_shader->enable();

	//Set uniforms
	sphere_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	sphere_shader->setUniform("u_camera_position", camera->eye);
	sphere_shader->setTexture("u_texture", sphere_texture, 0);
	sphere_shader->setMatrix44("u_model", sphere_model);
	sphere_shader->setUniform("u_color", Vector4(1.f, 1.f, 1.f, 1.f));
	sphere_shader->setUniform("u_time", (float)getTime());

	//Render bounding box
	sphere_mesh->render(GL_TRIANGLES);

	//Disable shader
	sphere_shader->disable();

	//Reset flags
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Renderer::setSceneUniforms(Shader* shader)
{
	//Shadow Atlas
	if (scene->shadow_atlas)
		shader->setTexture("u_shadow_atlas", scene->shadow_atlas, 8);

	//Upload scene uniforms
	shader->setVector3("u_ambient_light", scene->ambient_light);
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_time", (float)getTime());
	shader->setUniform("u_num_shadows", (float)scene->num_shadows);
}

//Render a draw call
void Renderer::renderDrawCall(Shader* shader, RenderCall* rc, Camera* camera)
{
	//In case there is nothing to do
	if (!rc->mesh || !rc->mesh->getNumVertices() || !rc->material)
		return;
	assert(glGetError() == GL_NO_ERROR);

	//Material factors
	Vector3 albedo_factor = rc->material->albedo_factor;
	Vector3 specular_factor = rc->material->specular_factor;
	Vector3 occlusion_factor = rc->material->occlusion_factor;
	Vector3 emissive_factor = rc->material->emissive_factor;

	//Textures
	Texture* albedo_texture = rc->material->albedo_texture.texture;
	Texture* specular_texture = rc->material->specular_texture.texture;
	Texture* normal_texture = rc->material->normal_texture.texture;
	Texture* occlusion_texture = rc->material->occlusion_texture.texture;
	Texture* metalness_texture = rc->material->metalness_texture.texture;
	Texture* roughness_texture = rc->material->roughness_texture.texture;
	Texture* omr_texture = rc->material->omr_texture.texture;
	Texture* emissive_texture = rc->material->emissive_texture.texture;

	//Texture booleans: Controls which texture are uploaded to the shader
	int textures[8] =
	{
		(albedo_texture != NULL), //0
		(specular_texture != NULL), //1
		(normal_texture != NULL), //2
		(occlusion_texture != NULL), //3
		(metalness_texture != NULL), //4
		(roughness_texture != NULL), //5
		(omr_texture != NULL), //6
		(emissive_texture != NULL)  //7
	};

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

	//Upload material factors
	shader->setVector3("u_albedo_factor", albedo_factor);
	shader->setVector3("u_specular_factor", specular_factor);
	shader->setVector3("u_occlusion_factor", occlusion_factor);
	shader->setVector3("u_emissive_factor", emissive_factor);

	//Upload textures
	if (textures[0]) shader->setTexture("u_albedo_texture", albedo_texture, 0);
	if (textures[1]) shader->setTexture("u_specular_texture", specular_texture, 1);
	if (textures[2]) shader->setTexture("u_normal_texture", normal_texture, 2);
	if (textures[3]) shader->setTexture("u_occlusion_texture", occlusion_texture, 3);
	if (textures[4]) shader->setTexture("u_metalness_texture", metalness_texture, 4);
	if (textures[5]) shader->setTexture("u_roughness_texture", roughness_texture, 5);
	if (textures[6]) shader->setTexture("u_omr_texture", omr_texture, 6);
	if (textures[7]) shader->setTexture("u_emissive_texture", emissive_texture, 7);

	//Upload the texture array of booleans
	shader->setUniform1Array("u_textures", (int*)&textures[0], 8);

	//Upload entity uniforms
	shader->setMatrix44("u_model", rc->model);
	shader->setUniform("u_color", rc->material->albedo_factor);
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

			//Check the visibility of the light
			if (!light->visible)
				continue;

			//General light properties
			lights_position[j] = light->model.getTranslation();
			lights_color[j] = light->color;
			lights_intensity[j] = light->intensity;
			lights_max_distance[j] = light->max_distance;

			//Specific light properties
			switch (light->light_type)
			{
			case(LightEntity::LightType::POINT_LIGHT):
				lights_type[j] = light->light_type;
				break;
			case (LightEntity::LightType::SPOT_LIGHT):
				spots_direction[j] = light->model.rotateVector(Vector3(0, 0, -1));
				spots_cone[j] = Vector2(light->cone_exp, cos(light->cone_angle * DEG2RAD));
				lights_type[j] = light->light_type;
				break;
			case (LightEntity::LightType::DIRECTIONAL_LIGHT):
				directionals_front[j] = light->model.rotateVector(Vector3(0, 0, -1));
				lights_type[j] = light->light_type;
				break;
			}

			//Shadow properties
			if (scene->shadow_atlas && light->cast_shadows)
			{
				cast_shadows[j] = 1;
				shadows_index[j] = (float)light->shadow_index;
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

//Multipass lighting
void Renderer::MultiPassLoop(Shader* shader, Mesh* mesh)
{
	//Blending support
	glDepthFunc(GL_LEQUAL);

	//Multi pass lighting
	for (int i = 0; i < scene->lights.size(); i++) {

		if (i == 0) shader->setUniform("u_last_iteration", 0);

		if (i == 1)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			shader->setUniform("u_ambient_light", Vector3());//reset the ambient light
		}
		if (i == scene->lights.size() - 1) shader->setUniform("u_last_iteration", 1);

		//Current light
		LightEntity* light = scene->lights[i];

		//Check the visibility of the light
		if (!light->visible)
			continue;			

		//Light uniforms
		shader->setUniform("u_light_position", light->model.getTranslation());
		shader->setUniform("u_light_color", light->color);
		shader->setUniform("u_light_intensity", light->intensity);
		shader->setUniform("u_light_max_distance", light->max_distance);

		//Specific light uniforms
		switch (light->light_type)
		{
		case(LightEntity::LightType::POINT_LIGHT):
			shader->setUniform("u_light_type", 0);
			break;
		case (LightEntity::LightType::SPOT_LIGHT):
			shader->setVector3("u_spot_direction", light->model.rotateVector(Vector3(0, 0, -1)));
			shader->setUniform("u_spot_cone", Vector2(light->cone_exp, cos(light->cone_angle * DEG2RAD)));
			shader->setUniform("u_light_type", 1);
			break;
		case (LightEntity::LightType::DIRECTIONAL_LIGHT):
			shader->setVector3("u_directional_front", light->model.rotateVector(Vector3(0, 0, -1)));
			shader->setUniform("u_area_size", light->area_size);
			shader->setUniform("u_light_type", 2);
			break;
		}

		//Shadow uniforms
		if (scene->shadow_atlas && light->cast_shadows)
		{
			shader->setUniform("u_cast_shadows", 1);
			shader->setUniform("u_shadow_index", (float)light->shadow_index);
			shader->setUniform("u_shadow_bias", light->shadow_bias);
			shader->setMatrix44("u_shadow_vp", light->shadow_camera->viewprojection_matrix);
			shader->setUniform("u_num_shadows", (float)scene->num_shadows);
		}
		else
		{
			shader->setUniform("u_cast_shadows", 0);
		}

		//do the draw call that renders the mesh into the screen
		mesh->render(GL_TRIANGLES);
	}

	//disable shader
	shader->disable();

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
	shader->setMatrix44("u_model", rc->model);
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
				Shader* shader = Shader::Get("quad.vs", "linearize.fs");
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

//Cubemap texture
Texture* CubemapFromHDRE(const char* filename)
{
	HDRE* hdre = HDRE::Get(filename);
	if (!hdre)
		return NULL;

	Texture* texture = new Texture();
	if (hdre->getFacesf(0))
	{
		texture->createCubemap(hdre->width, hdre->height, (Uint8**)hdre->getFacesf(0),
			hdre->header.numChannels == 3 ? GL_RGB : GL_RGBA, GL_FLOAT);
		for (int i = 1; i < hdre->levels; ++i)
			texture->uploadCubemap(texture->format, texture->type, false,
				(Uint8**)hdre->getFacesf(i), GL_RGBA32F, i);
	}
	else
		if (hdre->getFacesh(0))
		{
			texture->createCubemap(hdre->width, hdre->height, (Uint8**)hdre->getFacesh(0),
				hdre->header.numChannels == 3 ? GL_RGB : GL_RGBA, GL_HALF_FLOAT);
			for (int i = 1; i < hdre->levels; ++i)
				texture->uploadCubemap(texture->format, texture->type, false,
					(Uint8**)hdre->getFacesh(i), GL_RGBA16F, i);
		}
	return texture;
}