#include "Transform.h"


Transform::Transform()
{
	scale = Vector3(1, 1, 1); //0, 0, 0 would make it invisible
	moved = false; //object in it's original position
}

Transform::~Transform()
{

}

void Transform::setTranslate(float x, float y, float z)
{
	translate.x += x;
	translate.y += y;
	translate.z += z;
}

void Transform::setRotation(float x, float y, float z)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
}

void Transform::setScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}
