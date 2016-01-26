#include "SpringBox.h"

SpringBox::SpringBox(float x, float y, float z, XMVECTOR position, float mass, bool fixed, XMVECTOR orientation, float stiffness)
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
			XMVectorSet(0.f, 0.f, prefix*(x*x + y*y), 0.f),
			XMVectorSet(0.f, 0.f, 0.f, 1.f));
		intertiaTensorInverse = XMMatrixInverse(&XMMatrixDeterminant(matrix), matrix);
	}

	XMVECTOR xLength = XMVectorSet(x, 0.f, 0.f, 0.f) / 2;
	XMVECTOR yLength = XMVectorSet(0.f, y, 0.f, 0.f) / 2;
	XMVECTOR zLength = XMVectorSet(0.f, 0.f, z, 0.f) / 2;

	this->angularMomentum = XMVECTOR();
	this->angularVelocity = XMVECTOR();
	this->torqueAccumulator = XMVECTOR();

	corners.push_back(new Point(-xLength - yLength - zLength, fixed));
	corners.push_back(new Point(xLength - yLength - zLength, fixed));
	corners.push_back(new Point(-xLength + yLength - zLength, fixed));
	corners.push_back(new Point(xLength + yLength - zLength, fixed));
	corners.push_back(new Point(-xLength - yLength + zLength, fixed));
	corners.push_back(new Point(xLength - yLength + zLength, fixed));
	corners.push_back(new Point(-xLength + yLength + zLength, fixed));
	corners.push_back(new Point(xLength + yLength + zLength, fixed));

	// front side
	edges.push_back(new Spring(corners[0], corners[1], stiffness));
	edges.push_back(new Spring(corners[1], corners[3], stiffness));
	edges.push_back(new Spring(corners[2], corners[3], stiffness));
	edges.push_back(new Spring(corners[0], corners[2], stiffness));
	// back side
	edges.push_back(new Spring(corners[4], corners[5], stiffness));
	edges.push_back(new Spring(corners[5], corners[7], stiffness));
	edges.push_back(new Spring(corners[7], corners[6], stiffness));
	edges.push_back(new Spring(corners[6], corners[4], stiffness));
	// springs in between
	edges.push_back(new Spring(corners[1], corners[5], stiffness));
	edges.push_back(new Spring(corners[3], corners[7], stiffness));
	edges.push_back(new Spring(corners[2], corners[6], stiffness));
	edges.push_back(new Spring(corners[0], corners[4], stiffness));

}

SpringBox::~SpringBox()
{
	corners.clear();
	edges.clear();
}