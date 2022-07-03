/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This contains several functions that can be useful when programming your game.
*/
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <vector>

#include "includes.h"
#include "framework.h"
#include "extra/cJSON.h"

//General functions **************
long getTime();
bool readFile(const std::string& filename, std::string& content);
bool readFileBin(const std::string& filename, std::vector<unsigned char>& buffer);

//generic purposes fuctions
void drawGrid();
bool drawText(float x, float y, std::string text, Vector3 c, float scale = 1);

//check opengl errors
bool checkGLErrors();

std::string getPath();
void stdlog(std::string str);

Vector2 getDesktopSize( int display_index = 0 );

std::vector<std::string> tokenize(const std::string& source, const char* delimiters, bool process_strings = false);
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
std::string join(std::vector<std::string>& strings, const char* delim);

std::string getGPUStats();
void drawGrid();

//Used in the MESH and ANIM parsers
char* fetchWord(char* data, char* word);
char* fetchFloat(char* data, float& f);
char* fetchMatrix44(char* data, Matrix44& m);
char* fetchEndLine(char* data);
char* fetchBufferFloat(char* data, std::vector<float>& vector, int num = 0);
char* fetchBufferVec3(char* data, std::vector<Vector3>& vector);
char* fetchBufferVec2(char* data, std::vector<Vector2>& vector);
char* fetchBufferVec3u(char* data, std::vector<unsigned int>& vector);
char* fetchBufferVec4ub(char* data, std::vector<Vector4ub>& vector);
char* fetchBufferVec4(char* data, std::vector<Vector4>& vector);

//Read JSON
bool readJSONBoolean(cJSON* obj, const char* name, float default_value);
float readJSONNumber(cJSON* obj, const char* name, float default_value);
std::string readJSONString(cJSON* obj, const char* name, const char* default_str);
bool readJSONFloatVector(cJSON* obj, const char* name, std::vector<float>& dst);
bool readJSONBooleanVector(cJSON* obj, const char* name, std::vector<bool>& dst);
Vector3 readJSONVector3(cJSON* obj, const char* name, Vector3 default_value);
Vector4 readJSONVector4(cJSON* obj, const char* name);
cJSON* readJSONArrayItem(cJSON* obj, const char* name, int index);

//Populate JSON
bool populateJSONBooleanArray(cJSON* arr, std::vector<bool>& vtr);
bool populateJSONStringArray(cJSON* arr, std::vector<std::string>& vtr);
bool populateJSONIntArray(cJSON* arr, int* vtr, int vtr_size);
bool populateJSONIntArray(cJSON* arr, std::vector<int>& vtr);
bool populateJSONFloatArray(cJSON* arr, float* vtr, int vtr_size);
bool populateJSONFloatArray(cJSON* arr, std::vector<float>& vtr);

//Write JSON
void writeJSONBoolean(cJSON* obj, const char* name, bool boolean);
void writeJSONNumber(cJSON* obj, const char* name, float number);
void writeJSONString(cJSON* obj, const char* name, std::string str);
void writeJSONBooleanVector(cJSON* obj, const char* name, const int* vtr, int vtr_size);
void writeJSONBooleanVector(cJSON* obj, const char* name, std::vector<bool>& vtr);
void writeJSONIntVector(cJSON* obj, const char* name, const int* vtr, int vtr_size);
void writeJSONIntVector(cJSON* obj, const char* name, std::vector<int>& vtr);
void writeJSONFloatVector(cJSON* obj, const char* name, const float* vtr, int vtr_size);
void writeJSONFloatVector(cJSON* obj, const char* name, std::vector<float>& vtr);
void writeJSONVector3(cJSON* obj, const char* name, Vector3& vtr);
void writeJSONVector4(cJSON* obj, const char* name, Vector4& vtr);

//Replace JSON
void replaceJSONBoolean(cJSON* obj, const char* name, bool boolean);
void replaceJSONNumber(cJSON* obj, const char* name, float number);
void replaceJSONString(cJSON* obj, const char* name, const char* string);
void replaceJSONBooleanVector(cJSON* obj, const char* name, std::vector<bool>& vtr);
void replaceJSONFloatVector(cJSON* obj, const char* name, std::vector<float>& vtr);
void replaceJSONVector3(cJSON* obj, const char* name, Vector3 vector);
void replaceJSONVector4(cJSON* obj, const char* name, Vector4 vector);

//Custom methods
float computeDegrees(Vector2 a, Vector2 b);
float sign(float num);
float angleBetween(Vector3 vtr1, Vector3 vtr2); //Angle between vtr1 and vtr2
void addOffset(float& angle_in_rad, float offset_in_rad); //Maps angle + offset to [0,2pi] domain



#endif
