#ifndef STAGE_H
#define STAGE_H

#include <iostream>

enum class STAGE_ID {
	INTRO,
	TUTORIAL,
	PLAY,
	DIED,
	FINAL
};

class LoadStage {
public:
	static void render();
};

class IntroStage {
public:
	static void render();
	static STAGE_ID update(double seconds_elapsed);
};

class TutorialStage {
public:
	static void render();
	static STAGE_ID update(double seconds_elapsed);
};

class PlayStage {
public:
	static void render();
	static STAGE_ID update(double seconds_elapsed);
};

class DiedStage {
public:
	static void render();
	static STAGE_ID update(double seconds_elapsed);
};

class FinalStage {
public:
	static void render();
	static STAGE_ID update(double seconds_elapsed);
};

#endif