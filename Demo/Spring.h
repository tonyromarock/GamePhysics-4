#pragma once

#include <DirectXMath.h>
#include "Point.h"

using namespace DirectX;

class Spring {
public:
	Point* point1;
	Point* point2;

	float org_length; // spring length when idle
	float stiffness;
	static float* damping;	
	XMVECTOR forces;

	Spring();
	Spring(Point* point1, Point* point2, float stiffness);
	Spring(Point* point1, Point* point2, float o_length, float stiffness);
	~Spring();

	void computeSpringForces();
	void computeSpringForcesTMP();
	void addDamping();
	void addDampingTMP();
};