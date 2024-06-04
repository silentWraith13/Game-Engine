#pragma once
#include "Engine/Math/Vec2.hpp"
#include"Engine/Math/MathUtils.hpp"
#include "Engine/Math/Shape2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct OBB2D : public Shape2D
{
public:
	void GetCornerPoints(Vec2* out_fourCornerWorldPositions) const;
	void RotateAboutCenter(float rotationDeltaDegrees);
	void RotateAboutCorner(float rotationDegrees, Vec2 const& rotationPoint);
	Vec2 GetLocalPosForWorldPos(Vec2 worldPos) const;
	Vec2 GetWorldPosForLocalPos(Vec2 localPos) const;

	void Translate(Vec2 const& newCenter);

public:
	Vec2    m_center = Vec2(0.f,0.f);
	Vec2    m_iBasisNormal = Vec2(1.f, 0.f);
	Vec2    m_halfDimensions;
	Mat44   m_localToWorldMatrix;
	Mat44   m_worldToLocalMatrix;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------