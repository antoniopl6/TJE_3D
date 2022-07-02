#pragma once
#include "texture.h"
#include "material.h"
#include "entity.h"
#include "scene.h"

using namespace std;

class cMTL
{
public:
	//Current scene
	Scene* scene;
		
	//Constructor
	cMTL(Scene* scene);

	//Parser: Parses MTL to a list of Objects 
	vector<ObjectEntity*> Parse(string root, string asset);

	//Support methods
	bool GetBoolean(string buffer);
	string GetString(string buffer);
	float GetNumber(string buffer);
	Vector3 GetVector3(string buffer);
	vector<float> GetVector(string buffer);
	void replaceSlash(string& str);
	
};
