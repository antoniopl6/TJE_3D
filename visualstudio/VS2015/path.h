#ifndef PATH_H
#define PATH_H

#pragma once

#define MAX_STEPS 100

class Path {
	int startx; //Initial position
	int starty;
	int targetx; //Final position
	int targety;
	Path* nextPath;

	int output[MAX_STEPS]; //Store the positions to pass to reach the target one
	

};

#endif