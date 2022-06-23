#pragma once
#include "texture.h"
#include "material.h"
#include "entity.h"

using namespace std;

class cMTL
{
	//Parser: Parses MTL to a list of Objects 
	vector<ObjectEntity*> Parse(string root, string asset);

	//Support methods
	boolean GetBoolean(string buffer);
	string GetString(string buffer);
	float GetNumber(string buffer);
	Vector3 GetVector3(string buffer);
	vector<float> GetVector(string buffer);
	
};
