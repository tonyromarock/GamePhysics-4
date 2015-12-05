#pragma once

#include <DirectXMath.h>
#include "Point.h"

using namespace DirectX;

class RigidBody 
{
public:
	Point centerOfMass = Point(XMVectorSet(0.f, 0.f, 0.f, 0.f), true);
	float massInverse;
	XMVECTOR position;
	XMVECTOR velocity;
	XMVECTOR length;

	XMMATRIX intertiaTensorInverse;
	XMVECTOR orientation;
	XMVECTOR angularVelocity;
	XMVECTOR angularMomentum;
	XMVECTOR torqueAccumulator;
	XMMATRIX transform;

	void addTorque(XMVECTOR torque, XMVECTOR position);
	void clearTorque();
	void addVelocity(float x, float y, float z);

};