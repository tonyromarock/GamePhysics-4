#include "Spring.h"

// help function
float getDistance(XMVECTOR* p, XMVECTOR* q);

float* Spring::damping = nullptr;

Spring::Spring(Point* point1, Point* point2, float stiffness) : point1(point1), point2(point2), stiffness(stiffness)
{
	org_length = getDistance(&point1->position, &point2->position);
	forces = XMVECTOR();
}

Spring::Spring(Point* point1, Point* point2, float o_length, float stiffness) : point1(point1), point2(point2), org_length(o_length), stiffness(stiffness)
{
	forces = XMVECTOR();
}

Spring::~Spring(){}

void Spring::computeSpringForces()
{
	float curr_length = getDistance(&point1->position, &point2->position);
	float springForce = (-1 * stiffness) * (curr_length - org_length);
	forces = XMVectorSubtract(point1->position, point2->position);
	forces = XMVectorScale(forces, 1.f / curr_length);
	forces = XMVectorScale(forces, springForce);

	point1->addIntF(forces);
	point2->addIntF(XMVectorScale(forces, -1.f));
}

// same only with xtmp 
void Spring::computeSpringForcesTMP()
{
	float curr_length = getDistance(&point1->xtmp, &point2->xtmp);
	float springForce = (-1 * stiffness) * (curr_length - org_length);
	forces = XMVectorSubtract(point1->xtmp, point2->xtmp);
	forces = XMVectorScale(forces, 1.f / curr_length);
	forces = XMVectorScale(forces, springForce);

	point1->addIntF(forces);
	point2->addIntF(XMVectorScale(forces, -1.f));
}

void Spring::addDamping()
{
	XMVECTOR dampPoint1 = XMVectorScale(point1->velocity, -*damping);
	XMVECTOR dampPoint2 = XMVectorScale(point2->velocity, -*damping);

	point1->addIntF(dampPoint1);
	point2->addIntF(dampPoint2);
}

// same with vtmp
void Spring::addDampingTMP()
{
	XMVECTOR dampPoint1 = XMVectorScale(point1->vtmp, -*damping);
	XMVECTOR dampPoint2 = XMVectorScale(point2->vtmp, -*damping);

	point1->addIntF(dampPoint1);
	point2->addIntF(dampPoint2);
}



float getDistance(XMVECTOR* p, XMVECTOR* q)
{
	XMVECTOR diff = *p - *q;
	diff = XMVector3Length(diff); // see doc: solution written in each component
	return XMVectorGetByIndex(diff, 0);
}

