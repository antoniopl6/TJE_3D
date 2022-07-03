#include "utils.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "includes.h"

#include "game.h"
#include "camera.h"
#include "shader.h"
#include "mesh.h"

#include "extra/stb_easy_font.h"

#include <sstream>
#include <string>
#include <iostream>

long getTime()
{
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (int)(tv.tv_sec * 1000 + (tv.tv_usec / 1000));
#endif
}


//this function is used to access OpenGL Extensions (special features not supported by all cards)
void* getGLProcAddress(const char* name)
{
	return SDL_GL_GetProcAddress(name);
}

//Retrieve the current path of the application
#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

std::string getPath()
{
	std::string fullpath;
	// ----------------------------------------------------------------------------
	// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
	{
		// error!
	}
	CFRelease(resourcesURL);
	chdir(path);
	fullpath = path;
#else
	char cCurrentPath[1024];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		return "";

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	fullpath = cCurrentPath;

#endif    
	return fullpath;
}

template <typename T>
std::string to_string(T value)
{
	//create an output string stream
	std::ostringstream os;

	//throw the value into the string stream
	os << value;

	//convert the string stream into a string and return
	return os.str();
}

bool readFile(const std::string& filename, std::string& content)
{
	content.clear();

	long count = 0;

	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp == NULL)
	{
		std::cerr << "::readFile: file not found " << filename << std::endl;
		return false;
	}

	fseek(fp, 0, SEEK_END);
	count = ftell(fp);
	rewind(fp);

	content.resize(count);
	if (count > 0)
	{
		count = fread(&content[0], sizeof(char), count, fp);
	}
	fclose(fp);

	return true;
}

bool readFileBin(const std::string& filename, std::vector<unsigned char>& buffer)
{
	buffer.clear();
	FILE* fp = nullptr;
	fp = fopen(filename.c_str(), "rb");
	if (fp == nullptr)
		return false;
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	buffer.resize(size);
	fread(&buffer[0], sizeof(char), buffer.size(), fp);
	fclose(fp);
	return true;
}

void stdlog(std::string str)
{
	std::cout << str << std::endl;
}

bool checkGLErrors()
{
#ifdef _DEBUG
	GLenum errCode;
	const GLubyte *errString;

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		std::cerr << "OpenGL Error: " << errString << std::endl;
		return false;
	}
#endif

	return true;
}

std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

std::string join(std::vector<std::string>& strings, const char* delim)
{
	std::string str;
	for (int i = 0; i < strings.size(); ++i)
		str += strings[i] + (i < strings.size() - 1 ? std::string(delim) : "");
	return str;
}

Vector2 getDesktopSize(int display_index)
{
	SDL_DisplayMode current;
	// Get current display mode of all displays.
	int should_be_zero = SDL_GetCurrentDisplayMode(display_index, &current);
	return Vector2((float)current.w, (float)current.h);
}


bool drawText(float x, float y, std::string text, Vector3 c, float scale)
{
	static char buffer[99999]; // ~500 chars
	int num_quads;

	if (scale == 0)
		return true;

	x /= scale;
	y /= scale;

	if (Shader::current)
		Shader::current->disable();

	num_quads = stb_easy_font_print(x, y, (char*)(text.c_str()), NULL, buffer, sizeof(buffer));

	Matrix44 projection_matrix;
	projection_matrix.ortho(0, Game::instance->window_width / scale, Game::instance->window_height / scale, 0, -1, 1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(Matrix44().m);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(projection_matrix.m);

	glColor3f(c.x, c.y, c.z);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 16, buffer);
	glDrawArrays(GL_QUADS, 0, num_quads * 4);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

std::vector<std::string> tokenize(const std::string& source, const char* delimiters, bool process_strings)
{
	std::vector<std::string> tokens;

	std::string str;
	size_t del_size = strlen(delimiters);
	const char* pos = source.c_str();
	char in_string = 0;
	unsigned int i = 0;
	while (*pos != 0)
	{
		bool split = false;

		if (!process_strings || (process_strings && in_string == 0))
		{
			for (i = 0; i < del_size && *pos != delimiters[i]; i++);
			if (i != del_size) split = true;
		}

		if (process_strings && (*pos == '\"' || *pos == '\''))
		{
			if (!str.empty() && in_string == 0) //some chars remaining
			{
				tokens.push_back(str);
				str.clear();
			}

			in_string = (in_string != 0 ? 0 : *pos);
			if (in_string == 0)
			{
				str += *pos;
				split = true;
			}
		}

		if (split)
		{
			if (!str.empty())
			{
				tokens.push_back(str);
				str.clear();
			}
		}
		else
			str += *pos;
		pos++;
	}
	if (!str.empty())
		tokens.push_back(str);
	return tokens;
}

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

std::string getGPUStats()
{
	GLint nTotalMemoryInKB = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &nTotalMemoryInKB);
	GLint nCurAvailMemoryInKB = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &nCurAvailMemoryInKB);
	if (glGetError() != GL_NO_ERROR) //unsupported feature by driver
	{
		nTotalMemoryInKB = 0;
		nCurAvailMemoryInKB = 0;
	}

	std::string str = "FPS: " + to_string(Game::instance->fps) + " DCS: " + to_string(Mesh::num_meshes_rendered) + " Tris: " + to_string(long(Mesh::num_triangles_rendered * 0.001)) + "Ks  VRAM: " + to_string(int((nTotalMemoryInKB - nCurAvailMemoryInKB) * 0.001)) + "MBs / " + to_string(int(nTotalMemoryInKB * 0.001)) + "MBs";
	Mesh::num_meshes_rendered = 0;
	Mesh::num_triangles_rendered = 0;
	return str;
}

Mesh* grid = NULL;

void drawGrid()
{
	if (!grid)
	{
		grid = new Mesh();
		grid->createGrid(10);
	}

	glLineWidth(1);
	glEnable(GL_BLEND);
	glDepthMask(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Shader* grid_shader = Shader::getDefaultShader("grid");
	grid_shader->enable();
	Matrix44 m;
	m.translate(floor(Camera::current->eye.x / 100.0)*100.0f, 0.0f, floor(Camera::current->eye.z / 100.0f)*100.0f);
	grid_shader->setUniform("u_color", Vector4(0.7, 0.7, 0.7, 0.7));
	grid_shader->setUniform("u_model", m);
	grid_shader->setUniform("u_camera_position", Camera::current->eye);
	grid_shader->setUniform("u_viewprojection", Camera::current->viewprojection_matrix);
	grid->render(GL_LINES); //background grid
	glDisable(GL_BLEND);
	glDepthMask(true);
	grid_shader->disable();
}


char* fetchWord(char* data, char* word)
{
	int pos = 0;
	while (*data && *data != ',' && *data != '\n' && pos < 254) { word[pos++] = *data; data++; }
	word[pos] = 0;
	if (pos < 254)
		data++; //skip ',' or '\n'
	return data;
}

char* fetchFloat(char* data, float& v)
{
	char w[255];
	data = fetchWord(data, w);
	v = atof(w);
	return data;
}

char* fetchMatrix44(char* data, Matrix44& m)
{
	char word[255];
	for (int i = 0; i < 16; ++i)
	{
		data = fetchWord(data, word);
		m.m[i] = atof(word);
	}
	return data;
}

char* fetchEndLine(char* data)
{
	while (*data && *data != '\n') { data++; }
	if (*data == '\n')
		data++;
	return data;
}

char* fetchBufferFloat(char* data, std::vector<float>& vector, int num)
{
	int pos = 0;
	char word[255];
	if (num)
		vector.resize(num);
	else //read size with the first number
	{
		data = fetchWord(data, word);
		float v = atof(word);
		assert(v);
		vector.resize(v);
	}

	int index = 0;
	while (*data != 0) {
		if (*data == ',' || *data == '\n')
		{
			if (pos == 0)
			{
				data++;
				continue;
			}
			word[pos] = 0;
			float v = atof(word);
			vector[index++] = v;
			if (*data == '\n' || *data == 0)
			{
				if (*data == '\n')
					data++;
				return data;
			}
			data++;
			if (index >= vector.size())
				return data;
			pos = 0;
		}
		else
		{
			word[pos++] = *data;
			data++;
		}
	}

	return data;
}

char* fetchBufferVec3(char* data, std::vector<Vector3>& vector)
{
	//int pos = 0;
	std::vector<float> floats;
	data = fetchBufferFloat(data, floats);
	vector.resize(floats.size() / 3);
	memcpy(&vector[0], &floats[0], sizeof(float)*floats.size());
	return data;
}

char* fetchBufferVec2(char* data, std::vector<Vector2>& vector)
{
	//int pos = 0;
	std::vector<float> floats;
	data = fetchBufferFloat(data, floats);
	vector.resize(floats.size() / 2);
	memcpy(&vector[0], &floats[0], sizeof(float)*floats.size());
	return data;
}

char* fetchBufferVec3u(char* data, std::vector<unsigned int>& vector)
{
	int pos = 0;
	std::vector<float> floats;
	data = fetchBufferFloat(data, floats);
	vector.resize(floats.size());
	for (int i = 0; i < floats.size(); i++)
		vector[i] = (unsigned int)(floats[i]);
	return data;
}

char* fetchBufferVec4ub(char* data, std::vector<Vector4ub>& vector)
{
	//int pos = 0;
	std::vector<float> floats;
	data = fetchBufferFloat(data, floats);
	vector.resize(floats.size() / 4);
	for (int i = 0; i < floats.size(); i += 4)
		vector[i / 4].set(floats[i], floats[i + 1], floats[i + 2], floats[i + 3]);
	return data;
}

char* fetchBufferVec4(char* data, std::vector<Vector4>& vector)
{
	//int pos = 0;
	std::vector<float> floats;
	data = fetchBufferFloat(data, floats);
	vector.resize(floats.size() / 4);
	memcpy(&vector[0], &floats[0], sizeof(float)*floats.size());
	return data;
}

//Read JSON
bool readJSONBoolean(cJSON* obj, const char* name, float default_value)
{
	cJSON* str_json = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, name);
	if (!str_json)
		return default_value;
	else if (str_json->type == cJSON_False)
		return false;
	else if (str_json->type == cJSON_True)
		return true;
	else
		return default_value;
}

float readJSONNumber(cJSON* obj, const char* name, float default_value)
{
	cJSON* str_json = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, name);
	if (!str_json || str_json->type != cJSON_Number)
		return default_value;
	return str_json->valuedouble;
}

std::string readJSONString(cJSON* obj, const char* name, const char* default_str)
{
	cJSON* str_json = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, name);
	if (!str_json || str_json->type != cJSON_String)
		return default_str;
	return str_json->valuestring;
}

bool readJSONFloatVector(cJSON* obj, const char* name, vector<float>& dst)
{
	cJSON* array_json = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, name);
	if (!array_json)
		return false;
	if (!cJSON_IsArray(array_json))
		return false;

	dst.resize(cJSON_GetArraySize(array_json));
	for (int i = 0; i < dst.size(); ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(array_json, i);
		if (value_json)
			dst[i] = value_json->valuedouble;
		else
			dst[i] = 0;
	}

	return true;
}

bool readJSONBooleanVector(cJSON* obj, const char* name, std::vector<bool>& dst)
{
	cJSON* array_json = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, name);
	if (!array_json)
		return false;
	if (!cJSON_IsArray(array_json))
		return false;

	dst.resize(cJSON_GetArraySize(array_json));
	for (int i = 0; i < dst.size(); ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(array_json, i);
		if (value_json)
			dst[i] = value_json->valueint;
		else
			dst[i] = 1;
	}

	return true;
}

Vector3 readJSONVector3(cJSON* obj, const char* name, Vector3 default_value)
{
	std::vector<float> dst;
	if (readJSONFloatVector(obj, name, dst))
	{
		if (dst.size() == 3)
			return Vector3(dst[0], dst[1], dst[2]);
	}
	return default_value;
}

Vector4 readJSONVector4(cJSON* obj, const char* name)
{
	std::vector<float> dst;
	if (readJSONFloatVector(obj, name, dst))
	{
		if (dst.size() == 4)
			return Vector4(dst[0], dst[1], dst[2], dst[3]);
	}
	return Vector4();
}

cJSON* readJSONArrayItem(cJSON* obj, const char* name, int index)
{
	cJSON* array_json = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, name);
	if (!array_json)
		return NULL;
	if (!cJSON_IsArray(array_json))
		return NULL;

	return cJSON_GetArrayItem(array_json, index);
}

//Populate JSON
bool populateJSONBooleanArray(cJSON* arr, std::vector<bool>& vtr)
{
	if (!arr)
		return false;
	if (!cJSON_IsArray(arr))
		return false;

	vtr.resize(cJSON_GetArraySize(arr));
	for (int i = 0; i < vtr.size(); ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(arr, i);
		if (value_json)
			vtr[i] = value_json->valueint;
		else
			vtr[i] = 0;
	}

	return true;
}

bool populateJSONStringArray(cJSON* arr, std::vector<string>& vtr)
{
	if (!arr)
		return false;
	if (!cJSON_IsArray(arr))
		return false;

	vtr.resize(cJSON_GetArraySize(arr));
	for (int i = 0; i < vtr.size(); ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(arr, i);
		if (value_json)
			vtr[i] = value_json->valuestring;
		else
			vtr[i] = "";
	}

	return true;
}

bool populateJSONIntArray(cJSON* arr, int* vtr, int vtr_size)
{
	if (!arr)
		return false;
	if (!cJSON_IsArray(arr))
		return false;

	for (int i = 0; i < vtr_size; ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(arr, i);
		if (value_json)
			vtr[i] = value_json->valueint;
		else
			vtr[i] = 0;
	}

	return true;
}

bool populateJSONIntArray(cJSON* arr, std::vector<int>& vtr)
{
	if (!arr)
		return false;
	if (!cJSON_IsArray(arr))
		return false;

	vtr.clear();
	vtr.resize(cJSON_GetArraySize(arr));

	for (int i = 0; i < vtr.size(); ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(arr, i);
		if (value_json)
			vtr[i] = value_json->valueint;
		else
			vtr[i] = 0;
	}

	return true;
}

bool populateJSONFloatArray(cJSON* arr, float* vtr, int vtr_size)
{
	if (!arr)
		return false;
	if (!cJSON_IsArray(arr))
		return false;

	for (int i = 0; i < vtr_size; ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(arr, i);
		if (value_json)
			vtr[i] = value_json->valuedouble;
		else
			vtr[i] = 0.f;
	}

	return true;
}

bool populateJSONFloatArray(cJSON* arr, std::vector<float>& vtr)
{
	if (!arr)
		return false;
	if (!cJSON_IsArray(arr))
		return false;

	vtr.clear();
	vtr.resize(cJSON_GetArraySize(arr));

	for (int i = 0; i < vtr.size(); ++i)
	{
		cJSON* value_json = cJSON_GetArrayItem(arr, i);
		if (value_json)
			vtr[i] = value_json->valuedouble;
		else
			vtr[i] = 0.f;
	}

	return true;
}

//Write JSON
void writeJSONBoolean(cJSON* obj, const char* name, bool boolean)
{
	cJSON_AddBoolToObject(obj, name, boolean);
}

void writeJSONNumber(cJSON* obj, const char* name, float number)
{
	cJSON_AddNumberToObject(obj, name, number);
}

void writeJSONString(cJSON* obj, const char* name, std::string str)
{
	cJSON_AddStringToObject(obj, name, str.c_str());
}

void writeJSONBooleanVector(cJSON* obj, const char* name, const int* vtr, int vtr_size)
{
	cJSON* vector_json = cJSON_CreateIntArray(vtr, vtr_size);
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONBooleanVector(cJSON* obj, const char* name, std::vector<int>& vtr)
{
	const int* tmp_vtr = &vtr[0];
	cJSON* vector_json = cJSON_CreateIntArray(tmp_vtr, vtr.size());
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONIntVector(cJSON* obj, const char* name, const int* vtr, int vtr_size)
{
	cJSON* vector_json = cJSON_CreateIntArray(vtr, vtr_size);
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONIntVector(cJSON* obj, const char* name, vector<int>& vtr)
{

	const int* tmp_vtr = &vtr[0];
	cJSON* vector_json = cJSON_CreateIntArray(tmp_vtr, vtr.size());
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONFloatVector(cJSON* obj, const char* name, const float* vtr, int vtr_size)
{
	cJSON* vector_json = cJSON_CreateFloatArray(vtr, vtr_size);
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONFloatVector(cJSON* obj, const char* name, vector<float>& vtr)
{

	const float* tmp_vtr = &vtr[0];
	cJSON* vector_json = cJSON_CreateFloatArray(tmp_vtr, vtr.size());
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONVector3(cJSON* obj, const char* name, Vector3& vtr)
{
	const float tmp_vtr[3] = { vtr.x,vtr.y,vtr.z };
	cJSON* vector_json = cJSON_CreateFloatArray(tmp_vtr, 3);
	cJSON_AddItemToObject(obj, name, vector_json);
}

void writeJSONVector4(cJSON* obj, const char* name, Vector4& vtr)
{
	const float tmp_vtr[4] = { vtr.x,vtr.y,vtr.z, vtr.w };
	cJSON* vector_json = cJSON_CreateFloatArray(tmp_vtr, 4);
	cJSON_AddItemToObject(obj, name, vector_json);
}

//Replace JSON
void replaceJSONBoolean(cJSON* obj, const char* name, bool boolean)
{
	cJSON* new_item = cJSON_CreateBool(boolean);
	cJSON_ReplaceItemInObjectCaseSensitive(obj, name, new_item);
}

void replaceJSONNumber(cJSON* obj, const char* name, float number)
{

	cJSON* new_item = cJSON_CreateNumber(number);
	cJSON_ReplaceItemInObjectCaseSensitive(obj, name, new_item);

}

void replaceJSONString(cJSON* obj, const char* name, const char* string)
{
	cJSON* new_item = cJSON_CreateString(string);
	cJSON_ReplaceItemInObjectCaseSensitive(obj, name, new_item);
}

void replaceJSONBooleanVector(cJSON* obj, const char* name, std::vector<int>& vtr)
{
	const int* tmp_vtr = &vtr[0];
	cJSON* vector_json = cJSON_CreateIntArray(tmp_vtr, vtr.size());
	cJSON_AddItemToObject(obj, name, vector_json);
}

void replaceJSONFloatVector(cJSON* obj, const char* name, std::vector<float>& vtr)
{
	const float* tmp_vtr = &vtr[0];
	cJSON* new_item = cJSON_CreateFloatArray(tmp_vtr, vtr.size());
	cJSON_ReplaceItemInObjectCaseSensitive(obj, name, new_item);
}

void replaceJSONVector3(cJSON* obj, const char* name, Vector3 vector)
{
	const float new_array[3] = { vector.x,vector.y,vector.z };
	cJSON* new_item = cJSON_CreateFloatArray(new_array, 3);
	cJSON_ReplaceItemInObjectCaseSensitive(obj, name, new_item);
}

void replaceJSONVector4(cJSON* obj, const char* name, Vector4 vector)
{
	const float new_array[4] = { vector.x,vector.y,vector.z, vector.w };
	cJSON* new_item = cJSON_CreateFloatArray(new_array, 4);
	cJSON_ReplaceItemInObjectCaseSensitive(obj, name, new_item);
}


//Custom methods
float computeDegrees(Vector2 a, Vector2 b) {
	float modA = sqrt(a.x * a.x + a.y * a.y);
	float modB = sqrt(b.x * b.x + b.y * b.y);
	float dot = a.x * b.x + a.y * b.y;
	float degrees = acos(a.dot(b) / (modA * modB));
	float smallest = 1.17549e-38;
	//std::cout << "Grados a rotar" << degrees << std::endl;
	if (degrees > smallest)
		return degrees;
	return 0;
}

float sign(float num) {
	return num >= 0.0f ? 1.0f : -1.0f;
}

//This function doesn't work because the direction of the normal vector depends on the angle between vtr1 and vtr2 (above 180 degrees changes), which ironically we are looking for :)
float angleBetween(Vector3 vtr1, Vector3 vtr2)
{
	//Check normalization
	vtr1.normalize();
	vtr2.normalize();

	//Support vectors
	Vector3 normal_vector = vtr1.cross(vtr2).normalize();
	Vector3 side_vector = vtr1; // We do the angle between vtr1 and vtr2, not the other way around
	side_vector.rotate(-PI / 2, normal_vector);
	side_vector.normalize();

	//Dot products
	float side_product = side_vector.dot(vtr2);
	float angle_product = vtr1.dot(vtr2);
	
	//Length products
	float length_product = vtr1.length() * vtr2.length();

	//Angle result
	float angle = acos(angle_product / length_product);

	if (side_product > 0.f)
		return angle;
	else
		return 2 * PI - angle;
}

void addOffset(float& angle_in_rad, float offset_in_rad)
{
	angle_in_rad += offset_in_rad;
	angle_in_rad = angle_in_rad > 2 * PI ? angle_in_rad - 2 * PI : angle_in_rad;
}
