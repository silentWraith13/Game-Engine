#pragma once
#include "Engine/Math/Vec2.hpp"
struct Sphere2D
{
public:
	Vec2	m_center;
	float	m_radius = 0.0f;

public:
	Sphere2D() {};
	Sphere2D(Vec2 center, float radius);
	~Sphere2D() {};

};

