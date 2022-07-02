#include "stage.h"
#include "game.h"

void LoadStage::render() {

}

void IntroStage::render() {
	Game* g = Game::instance;
	Renderer* r = g->renderer;

	r->renderImage(r->introScene, g->window_width, g->window_height, g->window_width / 2, g->window_height / 2, Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 1), false);
	r->renderImage(r->title, 475, 150, g->window_width / 2, g->window_height / 2 - 100, Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 1));
	r->renderImage(r->continueX, 250, 40, g->window_width / 2, g->window_height - 50, Vector4(0, 0, 1, 1));

}
STAGE_ID IntroStage::update(double seconds_elapsed) {
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		return STAGE_ID::TUTORIAL;
	}
	return STAGE_ID::INTRO;
}

void TutorialStage::render() {
	Game* g = Game::instance;
	Renderer* r = g->renderer;

	r->renderImage(r->tutorialScene, g->window_width, g->window_height, g->window_width / 2, g->window_height / 2, Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 1), false);
	r->renderImage(r->note, 740, 400, g->window_width / 2, g->window_height / 2, Vector4(0, 0, 1, 1));
	r->renderImage(r->keyboard, 250, 250, g->window_width / 2 - 212, g->window_height / 2, Vector4(0, 0, 1, 1));
	r->renderImage(r->mouseTutorial, 100, 100, g->window_width / 2 - 42, g->window_height / 2, Vector4(0, 0, 1, 1));
	drawText(g->window_width / 2 - 212, g->window_height / 2 + 90, "Move the player", Vector3(0, 0, 0), 2);

	r->renderImage(r->keyboard_fe, 250, 250, g->window_width / 2 + 220, g->window_height / 2, Vector4(0, 0, 1, 1));
	drawText(g->window_width / 2 + 45, g->window_height / 2 + 90, "Turn on/off flashlight", Vector3(0, 0, 0), 2);
	drawText(g->window_width / 2 + 75, g->window_height / 2 - 40, "Collect Items", Vector3(0, 0, 0), 2);


	r->renderImage(r->continueX, 250, 40, g->window_width / 2, g->window_height - 50, Vector4(0, 0, 1, 1));

}
STAGE_ID TutorialStage::update(double seconds_elapsed) {
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		return STAGE_ID::PLAY;
	}
	return STAGE_ID::TUTORIAL;
}



void PlayStage::render() {

	Game* g = Game::instance;

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

	//Render the sound sphere for the editor
	if (g->entity_editor->selected_sound)
		g->renderer->renderSoundSphere(g->entity_editor->selected_sound);

	//Render GUIs
	g->renderer->renderGUIs();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
}

STAGE_ID PlayStage::update(double seconds_elapsed) {
	Game* g = Game::instance;
	
	if (!g->render_editor) 
	{
		//Update Main Character
		MainCharacterEntity* character = g->scene->main_character;

		//Update
		character->update(seconds_elapsed);

		if (character->bounding_box_trigger)
		{
			character->updateBoundingBox();
			character->bounding_box_trigger = false;

		}

		//Update Monster
		MonsterEntity* monster = g->scene->monster;

		//Update
		monster->update(g->elapsed_time);

		//Bounding box
		if (monster->bounding_box_trigger)
		{
			monster->updateBoundingBox();
			monster->bounding_box_trigger = false;
		}

		//Path finding IA
		if (monster->isInFollowRange(character)) {
			monster->updateFollow(g->elapsed_time, character->camera);
		}
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
	}

	//Update cameras
	switch (g->entity_editor->current_camera)
	{
	case(Editor3D::MAIN):
		g->scene->main_character->updateMainCamera(seconds_elapsed, g->mouse_speed, g->mouse_locked);
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

	//Win game condition
	if (g->scene->main_character->num_apples >= 10) {
		return STAGE_ID::FINAL;
	}


	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		if (g->scene->main_character->num_keys >= 1 && g->scene->hasDoorInRange()) {
			return STAGE_ID::FINAL;
		}
	}
	
	//Lose game condition
	if (g->scene->main_character->health == 0) {
		return STAGE_ID::DIED;
	}

	return STAGE_ID::PLAY;
	
}

void DiedStage::render() {
	Game* g = Game::instance;
	Renderer* r = g->renderer;

	r->renderImage(r->diedScene, g->window_width, g->window_height, g->window_width / 2, g->window_height / 2, Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 1), false);
	r->renderImage(r->diedTitle, 550, 150, g->window_width / 2, g->window_height / 2 - 100, Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 1));
	r->renderImage(r->exitX, 250, 40, g->window_width / 2, g->window_height - 50, Vector4(0, 0, 1, 1));
}
STAGE_ID DiedStage::update(double seconds_elapsed) {
	Game* g = Game::instance;
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		Game::instance->must_exit = true;
	}
	return STAGE_ID::DIED;
}

void FinalStage::render() {
	Game* g = Game::instance;
	Renderer* r = g->renderer;

	r->renderImage(r->finalScene, g->window_width, g->window_height, g->window_width / 2, g->window_height / 2, Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 1), false);
	r->renderImage(r->exitX, 250, 40, g->window_width / 2, g->window_height - 50, Vector4(0, 0, 1, 1));

}
STAGE_ID FinalStage::update(double seconds_elapsed) {
	
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		Game::instance->must_exit = true;
	}
	return STAGE_ID::FINAL;
}