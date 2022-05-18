
#ifndef ENTITY_H
#define ENTITY_H

#include "includes.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "audio.h"
#include "animation.h"
#include "scene.h"

class Entity {
public:
	Entity(Matrix44 model); //constructor
	virtual ~Entity() {}; //destructor

	//some attributes 
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
	BoundingBox bounding;
	BoundingBox boundingCatchRange;
	Animation* animRunning;
	Animation* animWalking;

	float speed = 30.f;
	bool isRunning;

	MonsterEntity(Mesh* mesh, Texture* texture, Shader* shader, Vector4 color, Matrix44 model);

	void render();
	//Metodos en IA
	void updateNoFollow(float elapsed_time);
	void updateFollow(float elapsed_time);
	bool isInCatchRange();
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
	bool isInPickRange(Vector3 playerModel);

};
class SoundEntity : public Entity {
public:
	Audio audio;

	SoundEntity(Audio audio, Matrix44 model);

	void playSample();
	void changeVol(float new_v);
};
#endif