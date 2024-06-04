#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Shape2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct LineSegment2 : public Shape2D
{
	LineSegment2();
	explicit LineSegment2(Vec2 const& boneStart, Vec2 const& boneEnd);
	~LineSegment2();
public:
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDegrees);
	Vec2 GetCenter() const;

public:
	Vec2 m_start;
	Vec2 m_end;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------