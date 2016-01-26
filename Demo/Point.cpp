#include "Point.h"

float* Point::f_Mass = nullptr;
float* Point::f_Gravity = nullptr;

Point::Point() 
{
	this->position = XMVECTOR();
	mass = *f_Mass;
	int_F = XMVECTOR();
	ext_F = XMVECTOR();
	xtmp = XMVECTOR();
	vtmp = XMVECTOR();
	velocity = XMVECTOR();
}

Point::Point(XMVECTOR position, bool fixed) :fixed(fixed)
{
	this->position = XMVectorSet(XMVectorGetByIndex(position, 0), XMVectorGetByIndex(position, 1), XMVectorGetByIndex(position, 2), 0.f);
	mass = *f_Mass;
	int_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	ext_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	xtmp = XMVectorSet(XMVectorGetByIndex(position, 0), XMVectorGetByIndex(position, 1), XMVectorGetByIndex(position, 2), 0.f);
	vtmp = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	velocity = XMVECTOR();
}

Point::~Point(){}

void Point::clearForces()
{
	int_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	ext_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

void Point::addGravity(float timeStep)
{
	ext_F += XMVectorSet(0.f, *f_Mass * *f_Gravity, 0.f, 0.f) * timeStep;
}

void Point::addIntF(XMVECTOR vec)
{
	int_F += vec;
}

void Point::addExtF(XMVECTOR vec)
{
	ext_F += vec;
}

XMVECTOR Point::getTotalForce()
{
	return int_F + ext_F;
}