#ifndef PATH_H
#define PATH_H

#include "includes.h"
#include "utils.h"
#include "pathfinders.h"
#include <iostream>
#include <fstream>

#define MAX_STEPS 100

class Point {
public:
	int startx; //Initial position
	int starty;
	//Point* nextPath;
	int path_steps;
	int output[MAX_STEPS]; //Store the positions to pass to reach the next point in scene
	Vector2 path[MAX_STEPS];

	Point(int startx, int starty);
	void SetPath(uint8* grid, int targetx, int targety, int W, int H);

};

class Route {
public:
	std::vector<Point*> route;
	uint8* grid;
	int W;
	int H;
	int currPoint = 0;
	const int tileSizeX = 100;
	const int tileSizeY = 100;

	Route(int W, int H, std::vector<Vector3> &points);
	Point* getClosestPoint(Vector3 translation); //Gets the closest route point to start path
	bool hasArrived(Vector3 translation); //Cheks if has arrived to the target point
	Vector3 getSceneVector(int x, int y);
	Vector2 getGridVector(int x, int y, int z);

};

#endif