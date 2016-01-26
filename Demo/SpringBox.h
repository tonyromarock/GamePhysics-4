#pragma once
#include "RigidBody.h"
#include "Point.h"
#include "Spring.h"
#include <vector>

using namespace DirectX;

class SpringBox : RigidBody
{
public:

	std::vector<Point*> corners;
	std::vector<Spring*> edges;

	// stiffness of the spring edges
	float stiffness;

	SpringBox(float x, float y, float z, XMVECTOR position, float mass, bool fixed, XMVECTOR orientation, float stiffness);
	~SpringBox();
};