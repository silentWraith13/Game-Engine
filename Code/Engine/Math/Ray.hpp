#pragma once
#include <Engine/Math/Vec3.hpp>

struct Ray
{
public:
	Ray(Vec3 startPosition, Vec3 direction);
	Vec3 m_startPosition;
	Vec3 m_normalizedDirection;
};