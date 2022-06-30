#include "stage.h"
#include "game.h"

void IntroStage::render() {
	Game* g = Game::instance;


}


void PlayStage::render() {
	Game* g = Game::instance;
	//Set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Check gl errors before starting
	checkGLErrors();

	//Render the scene
	switch (g->entity_editor->current_camera)
	{
	case(Editor3D::MAIN):
		g->main_camera->enable();
		g->renderer->renderScene(g->scene, g->main_camera);
		break;
	case(Editor3D::ENTITY):
		g->entity_editor->camera->enable();
		g->renderer->renderScene(g->scene, g->entity_editor->camera);
		break;
	}

	//Render GUIs
	g->renderer->renderGUIs();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(g->window);
}

STAGE_ID PlayStage::update(double seconds_elapsed) {
	Game* g = Game::instance;

	//Update Main Character
	MainCharacterEntity* character = g->scene->main_character;

	if (character->bounding_box_trigger)
	{
		character->updateBoundingBox();
		character->bounding_box_trigger = false;

	}

	//Update Monster
	MonsterEntity* monster = g->scene->monster;
	monster->update(g->elapsed_time);
	if (monster->bounding_box_trigger)
	{
		monster->updateBoundingBox();
		monster->bounding_box_trigger = false;
	}
	if (monster->isInFollowRange(character)) {
		monster->updateFollow(g->elapsed_time, character->camera);
	}
	//Path finding IA
	else {
		monster->followPath(g->elapsed_time);
	}


	//Update Objects
	for (int i = 0; i < g->scene->objects.size(); ++i)
	{
		ObjectEntity* object = g->scene->objects[i];
		if (object->bounding_box_trigger) {
			object->updateBoundingBox();
			object->bounding_box_trigger = false;
		}
	}

	//Update Lights
	for (int i = 0; i < g->scene->lights.size(); i++)
	{
		//TODO
	}

	//Update Sounds
	for (int i = 0; i < g->scene->sounds.size(); i++)
	{
		//TODO
	}

	//Update cameras
	switch (g->entity_editor->current_camera)
	{
	case(Editor3D::MAIN):
		character->update(seconds_elapsed);
		character->updateMainCamera(seconds_elapsed, g->mouse_speed, g->mouse_locked);

		break;
	case(Editor3D::ENTITY):
		g->entity_editor->updateCamera(seconds_elapsed, g->mouse_speed, g->mouse_locked);
		break;
	}

	//Render entity editor
	if (g->render_editor)
		g->entity_editor->render();


	//Save scene
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL) && Input::isKeyPressed(SDL_SCANCODE_S))
	{
		if (!g->scene_saved)
		{
			g->scene->save();
			g->scene_saved = true;
		}
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_X)) {
		if (character->num_keys >= 1 && g->scene->hasDoorInRange()) {
			return STAGE_ID::FINAL;
		}
	}
	if (character->health == 0) {
		return STAGE_ID::DIED;
	}
	return STAGE_ID::PLAY;
	
}