#include "RigidBody.h"

void RigidBody::addTorque(XMVECTOR force, XMVECTOR position)
{
	XMVECTOR product = XMVECTOR();
	product = XMVector3Cross(XMVectorSubtract(position, centerOfMass.position), force);
	torqueAccumulator += force * product;
}

void RigidBody::clearTorque()
{
	torqueAccumulator = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

void RigidBody::addVelocity(float x, float y, float z)
{
	this->velocity = XMVectorSet(x, y, z, 0.f);
}