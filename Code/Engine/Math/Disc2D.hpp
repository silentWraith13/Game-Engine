#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Shape2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Disc2D : public Shape2D
{
	Disc2D();
	~Disc2D();

	void SetCenter(Vec2 const& translation);
	void Translate(Vec2 const& translation);
	Vec2 GetCenter();

	float m_radius;
	Vec2  m_center;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------