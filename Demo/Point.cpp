#include "Point.h"

float* Point::f_Mass = nullptr;
float* Point::f_Gravity = nullptr;

Point::Point(XMVECTOR* position, bool fixed) :fixed(fixed)
{
	position = position;
	mass = *f_Mass;
	int_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	ext_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

Point::~Point(){}

void Point::clearForces()
{
	int_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	ext_F = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

void Point::addGravity()
{
	ext_F += XMVectorSet(0.f, *f_Mass * *f_Gravity, 0.f, 0.f);
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