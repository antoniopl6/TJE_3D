
#ifndef ENTITY_H
#define ENTITY_H

#include "includes.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "audio.h"
#include "animation.h"
#include "scene.h"

enum EntityType {
	MAIN = 0,
	MONSTER = 1,
	OBJECT = 2,
	LIGHT = 3
};

enum LightType {
	POINT = 0,
	SPOT = 1,
	DIRECTIONAL = 2
};

enum ObjectType {
	PICK_OBJECT = 1,
	RENDER_OBJECT = 2,
};

class Entity {
public:
	Entity(Matrix44 model); //constructor
	virtual ~Entity() {}; //destructor

	//some attributes 
	EntityType entity_type;
	std::string name;
	Matrix44 model;

	//methods overwritten by derived classes 
	virtual void render() {};
	virtual void update(float elapsed_time) {};

	//some useful methods...
	Vector3 getPosition() {};
};

class MonsterEntity: public Entity{
public:
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector4 color;
	//Bounding for colisions
	BoundingBox bounding;
	//Bounding to catch the character
	BoundingBox boundingCatchRange;
	//Bounding to see the character and start following
	BoundingBox boundingViewRange;
	Animation* animRunning;
	Animation* animWalking;

	float speed = 30.f;
	bool isRunning;
	bool isFollowing;

	MonsterEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, BoundingBox bounding, BoundingBox boundingCatchRange, BoundingBox boundingViewRange, Matrix44 model);

	void render();
	//Metodos en IA
	void updateNoFollow(float elapsed_time);
	void updateFollow(float elapsed_time);
	////
	bool isInViewRange(Matrix44 mainModel);
	bool isInCatchRange(Matrix44 mainModel);
};

class PickEntity : public Entity {
public:
	enum PickType {
		KEY = 1,
		RECOLECTABLE = 2,
	};
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector4 color;
	PickType type;
	BoundingBox boundingPickRange;

	PickEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, Matrix44 model, PickType type);

	//methods overwritten 
	void render();
	bool isInPickRange(Matrix44 mainModel);

};
class SoundEntity : public Entity {
public:
	Audio audio;

	SoundEntity(Audio audio, Matrix44 model);

	void playSample();
	void stopAudio();
	void changeVol(float new_v);
};
#endif