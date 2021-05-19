#pragma once

//| Name            | Akshay Sharma   |
//|---------------- | --------------- |
//| Student Number  | 7859678         |
//| Assignment      | project         |
//| Course          | COMP 4490       |
//| Instructor      | John Braico     |
//
// Header files for the Cube and Point helper clases

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp> 

typedef glm::vec4 point4;

class Point {
	glm::vec4 vertex;
	bool state;
	float density;

public:
	Point();

	Point(glm::vec4 vert, bool s, float density);

	point4* getPoint();

	bool getState();

	void setState(bool st);

	float getDensity();

	void setDensity(float d);

};

class Cube {
	//requires all the vertices and gives you the midpoint of the current cube
	Point* vertices[8];

public:
	Cube();

	bool setArray(int position, Point point);

	Point** getArray();

	point4 getBetweenPoint(int edge);

	int getCaseNumber();

	void updatePoints(std::vector<Point*> points, int startingPos, int numPerRow);
};

//class Noise {
//
//	static int perlin3D();
//};

