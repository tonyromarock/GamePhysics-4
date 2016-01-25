#include "Box.h"

Box::Box(float x, float y, float z, XMVECTOR position, float mass, bool fixed, XMVECTOR orientation)
{
	this->position = position;	// its the center position of the box
	this->velocity = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	length = XMVectorSet(x, y, z, 0.f);
	
	centerOfMass = Point(position, false);

	this->transform = XMMatrixTranslationFromVector(this->position);
	this->centerOfMass.fixed = fixed;
	this->orientation = XMQuaternionRotationRollPitchYawFromVector(orientation);

	if (fixed) 
	{
		centerOfMass.mass = 1.0f;
		massInverse = 0.0f;
		intertiaTensorInverse = XMMATRIX();
	}
	else 
	{
		centerOfMass.mass = mass;
		massInverse = 1.0f / mass;
		float prefix = (1.0f / 12.0f) * centerOfMass.mass;

		XMMATRIX matrix = XMMATRIX(XMVectorSet(prefix*(y*y + z*z), 0.f, 0.f, 0.f),
			XMVectorSet(0.f, prefix * (x*x + z*z), 0.f, 0.f),
			XMVectorSet(0.f, 0.f, prefix*(x*x + y*y),0.f),
			XMVectorSet(0.f, 0.f, 0.f, 1.f));
		intertiaTensorInverse = XMMatrixInverse(&XMMatrixDeterminant(matrix), matrix);
	}

	XMVECTOR xLength = XMVectorSet(x, 0.f, 0.f, 0.f) / 2;
	XMVECTOR yLength = XMVectorSet(0.f, y, 0.f, 0.f) / 2;
	XMVECTOR zLength = XMVectorSet(0.f, 0.f, z, 0.f) / 2;

	for (int i = 0; i < 8; i++) 
	{
		corners[0] = XMVECTOR();
	}

	this->angularMomentum = XMVECTOR();
	this->angularVelocity = XMVECTOR();
	this->torqueAccumulator = XMVECTOR();



	corners[0] = -xLength - yLength - zLength;
	corners[1] = xLength - yLength - zLength;
	corners[2] = -xLength + yLength - zLength;
	corners[3] = xLength + yLength - zLength;
	corners[4] = -xLength - yLength + zLength;
	corners[5] = xLength - yLength + zLength;
	corners[6] = -xLength + yLength + zLength;
	corners[7] = xLength + yLength + zLength;

}

Box::~Box()
{}