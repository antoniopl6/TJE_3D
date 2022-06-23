
#include "material.h"
#include "utils.h"
#include "includes.h"
#include "texture.h"

std::map<std::string, Material*> Material::sMaterials;

Material::Material()
{
	//Transparency
	alpha_mode = NO_ALPHA;
	alpha_cutoff = 0.5;
	two_sided = false;

	//Z-Range
	_zMin = 0.0f;
	_zMax = 1.0f;

	//Material factors
	occlusion_factor = 1.0f;
	roughness_factor = 1.0f;
	metallic_factor = 0.0f;
	emissive_factor.set(1.0f, 1.0f, 1.0f);

}

Material::Material(Texture* texture)
{
	//Transparency
	alpha_mode = NO_ALPHA;
	alpha_cutoff = 0.5;
	two_sided = false;

	//Z-Range
	_zMin = 0.0f;
	_zMax = 1.0f;

	//Material factors
	occlusion_factor = 1.0f;
	roughness_factor = 1.0f;
	metallic_factor = 0.0f;
	emissive_factor.set(1.0f, 1.0f, 1.0f);

	//Material textures
	color_texture.texture = texture;
}

Material::~Material()
{
	if (name.size())
	{
		auto it = sMaterials.find(name);
		if (it != sMaterials.end())
			sMaterials.erase(it);
	}
}

Material* Material::Get(const char* name)
{
	assert(name);
	std::map<std::string, Material*>::iterator it = sMaterials.find(name);
	if (it != sMaterials.end())
		return it->second;
	return NULL;
}

void Material::registerMaterial(const char* name)
{
	this->name = name;
	sMaterials[name] = this;

	// Ugly Hack for clouds sorting problem
	if (!strcmp(name, "Clouds"))
	{
		_zMin = 0.9f;
		_zMax = 1.0f;
	}
}

void Material::releaseMaterials()
{
	std::vector<Material *>mats;

	for (auto mp : sMaterials)
	{
		Material *m = mp.second;
		mats.push_back(m);
	}

	for (Material *m : mats)
	{
		delete m;
	}
	sMaterials.clear();
}

void Material::load(cJSON* material_json)
{
	//Check if entity has materials
	if (!material_json)
		return;

	//Transparency
	string transparency = readJSONString(material_json, "transparency", "NO_ALPHA");
	if (transparency == "NO_ALPHA") alpha_mode = AlphaMode::NO_ALPHA;
	else if(transparency == "MASK") alpha_mode = AlphaMode::MASK;
	else if(transparency == "BLEND") alpha_mode = AlphaMode::BLEND;
	alpha_cutoff = readJSONNumber(material_json, "alpha_cutoff", alpha_cutoff);
	two_sided = readJSONBoolean(material_json, "two_sided", two_sided);

	//Material factors
	occlusion_factor = readJSONVector3(material_json, "occlusion_factor", occlusion_factor);
	roughness_factor = readJSONVector3(material_json, "roughness_factor", roughness_factor);
	metallic_factor = readJSONVector3(material_json, "metallic_factor", metallic_factor);
	emissive_factor = readJSONVector3(material_json, "emissive_factor", emissive_factor);

	//Textures filenames
	string color_filename = readJSONString(material_json, "color_texture", "");
	string normal_filename = readJSONString(material_json, "normal_texture", "");
	string opacity_filename = readJSONString(material_json, "opacity_texture", "");
	string metallic_roughness_filename = readJSONString(material_json, "metallic_roughness_texture", "");
	string occlusion_filename = readJSONString(material_json, "occlusion_texture", "");
	string emissive_filename = readJSONString(material_json, "emissive_texture", "");

	//Material textures
	if(!color_filename.empty()) color_texture.texture = Texture::Get(color_filename.c_str());
	if(!normal_filename.empty()) normal_texture.texture = Texture::Get(normal_filename.c_str());
	if(!opacity_filename.empty()) opacity_texture.texture = Texture::Get(opacity_filename.c_str());
	if(!metallic_roughness_filename.empty()) metallic_roughness_texture.texture = Texture::Get(metallic_roughness_filename.c_str());
	if(!occlusion_filename.empty()) occlusion_texture.texture = Texture::Get(occlusion_filename.c_str());
	if(!emissive_filename.empty()) emissive_texture.texture = Texture::Get(emissive_filename.c_str());

}

void Material::save(cJSON* entity_json) 
{
	//Create JSON's material
	cJSON* material_json = cJSON_AddObjectToObject(entity_json, "material");

	//Transparency
	switch (alpha_mode)
	{
	case(AlphaMode::NO_ALPHA):
		writeJSONString(material_json, "transparency", "NO_ALPHA");
		break;
	case(AlphaMode::MASK):
		writeJSONString(material_json, "transparency", "MASK");
		break;
	case(AlphaMode::BLEND):
		writeJSONString(material_json, "transparency", "BLEND");
		break;
	}
	writeJSONNumber(material_json, "alpha_cutoff", alpha_cutoff);
	writeJSONNumber(material_json, "two_sided", two_sided);

	//Material factors
	writeJSONNumber(material_json, "occlusion_factor", occlusion_factor);
	writeJSONNumber(material_json, "roughness_factor", roughness_factor);
	writeJSONNumber(material_json, "metallic_factor", metallic_factor);
	writeJSONVector3(material_json, "emissive_factor", emissive_factor);

	//Material textures
	if(color_texture.texture) writeJSONString(material_json, "color_texture", color_texture.texture->filename);
	if(normal_texture.texture) writeJSONString(material_json, "normal_texture", normal_texture.texture->filename);
	if (opacity_texture.texture) writeJSONString(material_json, "opacity_texture", opacity_texture.texture->filename);
	if (metallic_roughness_texture.texture) writeJSONString(material_json, "metallic_roughness_texture", metallic_roughness_texture.texture->filename);
	if (occlusion_texture.texture) writeJSONString(material_json, "occlusion_texture", occlusion_texture.texture->filename);
	if (emissive_texture.texture) writeJSONString(material_json, "emissive_texture", emissive_texture.texture->filename);
	
}