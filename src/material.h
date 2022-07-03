#pragma once

#include "framework.h"
#include <cassert>
#include <map>
#include <string>
#include "extra/cJSON.h"

//forward declaration
class Mesh;
class Texture;

using namespace std;

enum AlphaMode {
	NO_ALPHA,
	MASK,
	BLEND
};

struct Sampler {
	Texture* texture;
	int uv_channel;

	Sampler() { texture = NULL; uv_channel = 0; }
};

//this class contains all info relevant of how something must be rendered
class Material {
public:
	//Name
	string name;

	//Parameters to control transparency
	AlphaMode alpha_mode;	//could be NO_ALPHA, MASK (alpha cut) or BLEND (alpha blend)
	float alpha_cutoff;		//pixels with alpha than this value shouldn't be rendered
	bool two_sided;			//render both faces of the triangles

	float _zMin, _zMax;       // Z-Range

	//Material factors
	Vector3 occlusion_factor;	//how occluded is the surface
	Vector3 albedo_factor;		//color of the surface
	Vector3 specular_factor;	//shinny points of the surface
	Vector3 emissive_factor;	//does this object emit light?

	//Material textures
	Sampler albedo_texture;		//base texture for color (must be modulated by the color factor)
	Sampler specular_texture;	// specular color
	Sampler normal_texture;		//normalmap
	Sampler occlusion_texture;	//which areas receive ambient light
	Sampler metalness_texture;	//how metallic is the surface
	Sampler roughness_texture;	//how smooth or rough is the surface
	Sampler omr_texture;		//occlusion, metallic and roughtness (in R, G and B)
	Sampler emissive_texture;	//emissive texture (must be modulated by the emissive factor)

	//Constructor and destructors
	Material();
	Material(Texture* texture);
	virtual ~Material();

	//Static manager to reuse materials
	static std::map<std::string, Material*> sMaterials;
	static Material* Get(const char* name);
	void print();
	void registerMaterial(const char* name);
	static void releaseMaterials();

	//JSON methods
	void load(cJSON* material_json);
	void save(cJSON* entity_json);
};
