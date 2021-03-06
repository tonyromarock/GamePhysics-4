#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Point {
public:
	static float* f_Mass;
	static float* f_Gravity;

	XMVECTOR position;
	XMVECTOR int_F;
	XMVECTOR ext_F;
	XMVECTOR velocity;

	bool fixed;
	float mass;

	// for midpoint xtmp and vtmp
	XMVECTOR xtmp;
	XMVECTOR vtmp;

	Point();
	Point(XMVECTOR position, bool fixed);
	~Point();
	void clearForces();
	void addGravity(float timeStep);
	void addIntF(XMVECTOR vec);
	void addExtF(XMVECTOR vec);
	XMVECTOR getTotalForce();
};