
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
	occlusion_factor.set(1.0f, 1.0f, 1.0f); //Ka
	albedo_factor.set(1.0f, 1.0f, 1.0f); //Kd
	specular_factor.set(1.0f, 1.0f, 1.0f); //Ks
	emissive_factor.set(0.0f, 0.0f, 0.0f); //Ke

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
	occlusion_factor.set(1.0f, 1.0f, 1.0f);
	albedo_factor.set(1.0f, 1.0f, 1.0f);
	specular_factor.set(1.0f, 1.0f, 1.0f);
	emissive_factor.set(1.0f, 1.0f, 1.0f);

	//Material textures
	albedo_texture.texture = texture;
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

void Material::print()
{
	cout << "Material" << endl << endl;
	cout << "AO factor: " << occlusion_factor.x << " " << occlusion_factor.y << " " << occlusion_factor.z << endl;
	cout << "Albedo factor: " << albedo_factor.x << " " << albedo_factor.y << " " << albedo_factor.z << endl;
	cout << "Specular factor: " << specular_factor.x << " " << specular_factor.y << " " << specular_factor.z << endl;
	cout << "Emissive factor: " << emissive_factor.x << " " << emissive_factor.y << " " << emissive_factor.z << endl;
	cout << "Albedo texture: " << ((albedo_texture.texture != NULL) ? albedo_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((specular_texture.texture != NULL) ? specular_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((normal_texture.texture != NULL) ? normal_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((occlusion_texture.texture != NULL) ? occlusion_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((metalness_texture.texture != NULL) ? metalness_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((roughness_texture.texture != NULL) ? roughness_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((omr_texture.texture != NULL) ? omr_texture.texture->filename : "") << endl;
	cout << "Albedo texture: " << ((emissive_texture.texture != NULL) ? emissive_texture.texture->filename : "") << endl;
	cout << "Alpha mode: " << ((alpha_mode == AlphaMode::BLEND) ? "Blend" : "Alpha") << endl;
	cout << "Two sided: " << two_sided << endl;

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
	std::vector<Material*>mats;

	for (auto mp : sMaterials)
	{
		Material* m = mp.second;
		mats.push_back(m);
	}

	for (Material* m : mats)
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
	else if (transparency == "MASK") alpha_mode = AlphaMode::MASK;
	else if (transparency == "BLEND") alpha_mode = AlphaMode::BLEND;
	alpha_cutoff = readJSONNumber(material_json, "alpha_cutoff", alpha_cutoff);
	two_sided = readJSONBoolean(material_json, "two_sided", two_sided);

	//Material factors
	occlusion_factor = readJSONVector3(material_json, "occlusion_factor", occlusion_factor);
	albedo_factor = readJSONVector3(material_json, "albedo_factor", albedo_factor);
	specular_factor = readJSONVector3(material_json, "specular_factor", specular_factor);
	emissive_factor = readJSONVector3(material_json, "emissive_factor", emissive_factor);

	//Textures filenames
	string albedo_filename = readJSONString(material_json, "albedo_texture", "");
	string specular_filename = readJSONString(material_json, "specular_texture", "");
	string normal_filename = readJSONString(material_json, "normal_texture", "");
	string occlusion_filename = readJSONString(material_json, "occlusion_texture", "");
	string metalness_filename = readJSONString(material_json, "metalness_texture", "");
	string roughness_filename = readJSONString(material_json, "roughness_texture", "");
	string omr_filename = readJSONString(material_json, "omr_texture", "");
	string emissive_filename = readJSONString(material_json, "emissive_texture", "");

	//Material textures
	if (!albedo_filename.empty()) albedo_texture.texture = Texture::Get(albedo_filename.c_str());
	if (!specular_filename.empty()) specular_texture.texture = Texture::Get(specular_filename.c_str());
	if (!normal_filename.empty()) normal_texture.texture = Texture::Get(normal_filename.c_str());
	if (!occlusion_filename.empty()) occlusion_texture.texture = Texture::Get(occlusion_filename.c_str());
	if (!metalness_filename.empty()) metalness_texture.texture = Texture::Get(metalness_filename.c_str());
	if (!roughness_filename.empty()) roughness_texture.texture = Texture::Get(roughness_filename.c_str());
	if (!omr_filename.empty()) omr_texture.texture = Texture::Get(omr_filename.c_str());
	if (!emissive_filename.empty()) emissive_texture.texture = Texture::Get(emissive_filename.c_str());

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
	writeJSONBoolean(material_json, "two_sided", two_sided);

	//Material factors
	writeJSONVector3(material_json, "occlusion_factor", occlusion_factor);
	writeJSONVector3(material_json, "albedo_factor", albedo_factor);
	writeJSONVector3(material_json, "specular_factor", specular_factor);
	writeJSONVector3(material_json, "emissive_factor", emissive_factor);

	//Material textures
	if (albedo_texture.texture) writeJSONString(material_json, "albedo_texture", albedo_texture.texture->filename);
	if (specular_texture.texture) writeJSONString(material_json, "specular_texture", specular_texture.texture->filename);
	if (normal_texture.texture) writeJSONString(material_json, "normal_texture", normal_texture.texture->filename);
	if (occlusion_texture.texture) writeJSONString(material_json, "occlusion_texture", occlusion_texture.texture->filename);
	if (metalness_texture.texture) writeJSONString(material_json, "metalness_texture", metalness_texture.texture->filename);
	if (roughness_texture.texture) writeJSONString(material_json, "roughness_texture", roughness_texture.texture->filename);
	if (omr_texture.texture) writeJSONString(material_json, "omr_texture", omr_texture.texture->filename);
	if (emissive_texture.texture) writeJSONString(material_json, "emissive_texture", emissive_texture.texture->filename);

}