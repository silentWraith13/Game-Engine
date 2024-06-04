#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Shape2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Capsule2 : public Shape2D
{
public:
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDegrees);
	Vec2 GetCenter() const;

public:
	LineSegment2 m_bone;
	
	float radius;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------