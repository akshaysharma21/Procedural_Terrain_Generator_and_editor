//| Name            | Akshay Sharma   |
//|---------------- | --------------- |
//| Student Number  | 7859678         |
//| Assignment      | project         |
//| Course          | COMP 4490       |
//| Instructor      | John Braico     |
//
//
// This file contains the implementation of the cube and Point class which are helper classes for the marching cubes algorithm and are defined in utils.h

#include "utils.h"
#include "common.h"
#include <vector>
#include "lookup.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef glm::vec4 point4;

//---------------------------------------------------
// cube class
//---------------------------------------------------

Cube::Cube() {
	for (int i = 0; i < 8; i++) {
		vertices[i] = &Point(point4(-1), false, -1);
	}
}

bool Cube::setArray(int position, Point point) {
	vertices[position] = &point;
	return true;
}

Point** Cube::getArray() {
	return vertices;
}

point4 Cube::getBetweenPoint(int edge) {
	//once you set up the density function, make this into a weighted average of the densities
	Point p1, p2;
	switch (edge) {
	case 0:
		p1 = *vertices[0];
		p2 = *vertices[1];
		break;
	case 1:
		p1 = *vertices[1];
		p2 = *vertices[2];
		break;
	case 2:
		p1 = *vertices[2];
		p2 = *vertices[3];
		break;
	case 3:
		p1 = *vertices[3];
		p2 = *vertices[0];
		break;
	case 4:
		p1 = *vertices[4];
		p2 = *vertices[5];
		break;
	case 5:
		p1 = *vertices[5];
		p2 = *vertices[6];
		break;
	case 6:
		p1 = *vertices[6];
		p2 = *vertices[7];
		break;
	case 7:
		p1 = *vertices[7];
		p2 = *vertices[4];
		break;
	case 8:
		p1 = *vertices[0];
		p2 = *vertices[4];
		break;
	case 9:
		p1 = *vertices[1];
		p2 = *vertices[5];
		break;
	case 10:
		p1 = *vertices[2];
		p2 = *vertices[6];
		break;
	case 11:
		p1 = *vertices[3];
		p2 = *vertices[7];
		break;
	}
	
	float p1Perc = abs(p1.getDensity())/ (abs(p1.getDensity()) + abs(p2.getDensity()));

	return (*p1.getPoint() * (1-p1Perc)) + (*p2.getPoint() * (p1Perc) );
}

int Cube::getCaseNumber() {
	int res = 0;
	for (int i = 0; i < 8; i++) {
		res += vertices[i]->getState() ? 1 * pow(2,i): 0;
	}

	return res;
}

void Cube::updatePoints(std::vector<Point*> points, int startingPos, int numPerRow) {
	vertices[0] = points[startingPos];
	vertices[1] = points[startingPos + 1];
	vertices[2] = points[startingPos + 1 + pow(numPerRow,2)];
	vertices[3] = points[startingPos + pow(numPerRow, 2)];
	vertices[4] = points[startingPos + numPerRow];
	vertices[5] = points[startingPos + numPerRow + 1];
	vertices[6] = points[startingPos + numPerRow + pow(numPerRow, 2) + 1];
	vertices[7] = points[startingPos + numPerRow + pow(numPerRow, 2)];
}

//---------------------------------------------------
// Point class
//---------------------------------------------------

Point::Point() {

}

Point::Point(point4 vert, bool s, float density) {
	this->vertex = vert;
	this->state = s;
	this->density = density;
}

point4* Point::getPoint() {
	return &vertex;
}

bool Point::getState() {
	return state;
}

void Point::setState(bool st) {
	this->state = st;
}

float Point::getDensity() {
	return this->density;
}

void Point::setDensity(float d) {
	this->density = d;
}


