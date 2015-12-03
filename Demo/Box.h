#pragma once
#include "RigidBody.h"

using namespace DirectX;

class Box : public RigidBody
{
	public:
		XMVECTOR corners[8];

	Box(float x, float y, float z, XMVECTOR position, float mass, bool fixed, XMVECTOR orientation);
	~Box();
};