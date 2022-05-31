#include "path.h"
#include "scene.h"

Scene* scene;

Point::Point(int startx, int starty) {
	this->startx = startx;
	this->starty = starty;
	path_steps = 0;

}

void Point::SetPath(uint8* grid, int targetx, int targety, int W, int H) {
	path_steps = AStarFindPathNoTieDiag(
		startx, starty, //origin (tienen que ser enteros)
		targetx, targety, //target (tienen que ser enteros)
		grid, //pointer to map data
		W, H, //map width and height
		output, //pointer where the final path will be stored
		100); //max supported steps of the final path
	if (path_steps != -1)
	{
		for (int i = 0; i < path_steps; ++i)
			path[i] = Vector2((output[i] % W), floor(output[i] / W));
	}
}

Route::Route(int W, int H, std::vector<Vector3> &points) {
	scene = Scene::instance;
	Vector3 coll;
	Vector3 collnorm;
	this->W = W;
	this->H = H;
	grid = new uint8[W * H];

	//Create grid of accesible points without colision
	for (size_t i = 0; i < W; i++)
	{
		for (size_t j = 0; j < H; j++)
		{
			bool value = !scene->hasCollision(getSceneVector(i, j), coll, collnorm);
			grid[i + j * W] = (int) value;
		}
	}

	//Adds points route to the vector route and initialize each point, setting his path from them to the next point
	Vector3 nextPoint;
	for (size_t i = 0; i < points.size(); i++)
	{
		Vector2 pos = getGridVector(points[i].x, points[i].y, points[i].z);
		Point* point = new Point(pos.x, pos.y);
		
		if (i < route.size() - 1)
			nextPoint = points[i + 1];
		else
			nextPoint = points[0];

		Vector2 nextPos = getGridVector(nextPoint.x, nextPoint.y, nextPoint.z);
		point->SetPath(grid, nextPos.x, nextPos.y, W, H);
		route.push_back(point);
	}

}

Point Route::getClosestPoint(Vector3 translation) {
	Vector2 currPos = getGridVector(translation.x, translation.y, translation.z);
	Point* closest = route[0];
	Vector2 toPos = currPos - Vector2(closest->startx, closest->starty);
	float lenght = toPos.length();
	float newLenght;

	for (size_t i = 1; i < route.size(); i++)
	{
		Point* point = route[i];
		Vector2 toPos = currPos - Vector2(point->startx, point->starty);
		newLenght = toPos.length();
		if (newLenght < lenght) {
			closest = point;
		}
	}
	return *closest;
}

Vector3 Route::getSceneVector(int x, int y) {

	return Vector3(x * W, 0, y * H);
}

Vector2 Route::getGridVector(int x, int y, int z) {

	return Vector2((int)x % W, floor((int)z / W));
}