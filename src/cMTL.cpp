#include "cMTL.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

//Parser
vector<ObjectEntity*> cMTL::Parse(string root, string asset)
{
	//Reader
	ifstream reader;
	reader.open(root + "\\" + asset + ".mtl", ios::out);

	if (reader)
	{
		//Notify the user the reader has succeded in opening the file	
		cout << "Success in opening" << endl;

		//Create string buffer for storing file lines
		string buffer;

		//List of objects and object index
		vector<ObjectEntity*> objects;
		ObjectEntity* current_object = NULL;

		while (!reader.eof())
		{
			//Get a new line and fill the buffer
			getline(reader, buffer);

			if (buffer.find("newmtl") != string::npos)
			{
				//Create new object
				ObjectEntity* current_object = new ObjectEntity();

				//Push the object to the list
				objects.push_back(current_object);

				//Get & assign name
				current_object->name = GetString(buffer);

			}
			else if (buffer.substr(0, 2) == "Ka")
			{
				//Current object check
				if (!current_object)
					continue;
				
				//Assign components
				current_object->material->occlusion_factor = GetVector3(buffer);
			}
			else if (buffer.substr(0, 2) == "Kd")
			{
				//Current object check
				if (!current_object)
					continue;

				//Assign components
				current_object->material->roughness_factor = GetVector3(buffer);
			}
			else if (buffer.substr(0, 2) == "Ks")
			{
				//Current object check
				if (!current_object)
					continue;

				//Assign components
				current_object->material->metallic_factor = GetVector3(buffer);
			}
			else if (buffer.substr(0, 2) == "Ke")
			{
				//Current object check
				if (!current_object)
					continue;

				//Assign components
				current_object->material->emissive_factor = GetVector3(buffer);
			}
			else if (buffer.find("alpha") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Alpha for blending
				bool alpha = GetBoolean(buffer);

				//Choose blending option
				alpha = true ? current_object->material->alpha_mode = AlphaMode::BLEND : current_object->material->alpha_mode = AlphaMode::NO_ALPHA;
				
			}
			else if (buffer.find("double_sided") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Inner and outer triangle sides render
				bool double_sided = GetBoolean(buffer);

				//Assign boolean
				current_object->material->two_sided = double_sided;				
			}
			else if (buffer.find("map_Ka") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());

			}
			else if (buffer.find("map_Kd") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_Ks") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_Ke") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_normal") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_occlusion") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_roughness") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_metalness") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("map_omr") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get ambient texture path
				string texture = GetString(buffer);

				//Texture name check
				if (texture.empty())
					continue;

				//Set path
				string path = root + "\\" + texture;

				//Get Texture
				current_object->material->occlusion_texture.texture = Texture::Get(path.c_str());
			}
			else if (buffer.find("local_model") != string::npos)
			{
				//Current object check
				if (!current_object)
					continue;

				//Get local model
				vector<float> local_model = GetVector(buffer);

				//Get & assign local model
				for (int i = 0; i < 16; ++i)
				{
					current_object->model.m[i] = local_model[i];
				}

			}

		}
		reader.close();
	}
	else
	{
		//Notify the user the file hasn't been opened	
		cout << "Issue in opening" << endl;
	}

	return 0;
}

//Support methods
boolean cMTL::GetBoolean(string buffer)
{
	return buffer.substr(buffer.find_first_of(' ') + 1, buffer.length()) == "true" ? true : false;
}

string cMTL::GetString(string buffer)
{
	return buffer.substr(buffer.find_first_of(' ') + 1, buffer.length());
}

float cMTL::GetNumber(string buffer)
{
	return strtof(buffer.substr(buffer.find_first_of(' ') + 1, buffer.length()).c_str(), NULL);
}

Vector3 cMTL::GetVector3(string buffer)
{
	//Output Vector
	Vector3 output;

	//Vector components
	stringstream components;
	components << buffer.substr(buffer.find_first_of(' ') + 1, buffer.length());
	string x, y, z;

	//Get components
	getline(components, x, ' ');
	getline(components, y, ' ');
	getline(components, z, ' ');

	//Assign components
	output = Vector3(strtof(x.c_str(), NULL), strtof(y.c_str(), NULL), strtof(z.c_str(), NULL));

	//Return vector
	return output;
}

vector<float> cMTL::GetVector(string buffer)
{
	//Output Vector
	vector<float> output;

	//Vector components
	stringstream components;
	components << buffer.substr(buffer.find_first_of(' ') + 1, buffer.length());
	string components_buffer;

	//Get and push components
	while (!components.eof())
	{
		getline(components, components_buffer, ' ');
		output.push_back(strtof(components_buffer.c_str(), NULL));
	}

	//Return vector
	return output;
}
