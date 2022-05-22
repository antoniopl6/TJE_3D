
#ifndef ENTITY_H
#define ENTITY_H
#pragma once 
#include "includes.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "audio.h"
#include "animation.h"
#include "pathfinders.h"
#include "input.h"
#include "camera.h"
#include <assert.h>

//forward declaration
class Camera;
class Mesh;
class Texture;



using namespace std;
enum EntityType {
	MAIN = 0,
	MONSTER = 1,
	OBJECT = 2,
	LIGHT = 3
};

enum ObjectType {
	PICK_OBJECT = 1,
	RENDER_OBJECT = 2,
};

enum LightType {
	LPOINT = 0,
	LSPOT = 1,
	LDIRECTIONAL = 2
};



class Entity {
public:

	Entity() {}; //constructor
	virtual ~Entity() {}; //destructor

	//Entity features
	string name;
	Matrix44 model;
	EntityType entity_type;

	//Methods overwritten by derived classes 
	virtual void update(float elapsed_time) {};
	void virtual updateBoundingBox() {};

	//Some useful methods...
	Vector3 getPosition();
};
class MainCharacterEntity : public Entity {
public:

	//Main features
	Camera* camera;
	Mesh* mesh;
	Texture* texture;
	BoundingBox world_bounding_box;

	//Triggers
	bool bounding_box_trigger;

	MainCharacterEntity(string name, Matrix44 model, Camera* camera, Mesh* mesh, Texture* texture); //constructor

	//Methods
	void updateMainCamera(double seconds_elapsed, float mouse_speed, bool mouse_locked);
	void updateMainMesh();
	void virtual updateBoundingBox();
	virtual void update(float elapsed_time) {};
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
	void update(float elapsed_time) {};
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
	void render() {};
	void update(float elapsed_time) {};
	bool isInPickRange(Matrix44 mainModel);

};
class SoundEntity : public Entity {
public:
	Audio audio;

	SoundEntity(Audio audio, Matrix44 model);

	void render() {};
	void update(float elapsed_time) {};
	void playSample();
	void stopAudio();
	void changeVol(float new_v);
};

class ObjectEntity : public Entity {
public:

	Mesh* mesh;
	Texture* texture;
	BoundingBox world_bounding_box;

	//Triggers
	bool bounding_box_trigger;

	ObjectEntity(string name, Matrix44 model, Mesh* mesh, Texture* texture); //constructor

	//Methods
	void virtual updateBoundingBox();
	virtual void update(float elapsed_time) {};

};

class LightEntity : public Entity {
public:

	//Light features
	Vector3 color;
	float intensity;
	float max_distance;
	LightType light_type;

	//Spot Light
	float cone_angle;
	float cone_exp;

	//Directional Light
	float area_size;

	//Shadows
	bool cast_shadows;
	int shadow_index;
	float shadow_bias;
	Camera* shadow_camera;

	//Methods
	LightEntity(string name, Vector3 color, float intensity, float max_distance, int light_type, float cone_angle, float cone_exp, float area_size, bool cast_shadows, float shadow_bias); //constructor
	virtual void update(float elapsed_time) {};
};


#endif