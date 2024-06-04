#include "Engine/Math/OBB2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OBB2D::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	Vec2 rightBasis = m_iBasisNormal.GetRotated90Degrees();
	Vec2 bottomLeft = m_center - m_halfDimensions.x * m_iBasisNormal - m_halfDimensions.y * rightBasis;
	Vec2 bottomRight = m_center + m_halfDimensions.x * m_iBasisNormal - m_halfDimensions.y * rightBasis;
	Vec2 topRight = m_center + m_halfDimensions.x * m_iBasisNormal + m_halfDimensions.y * rightBasis;
	Vec2 topLeft = m_center - m_halfDimensions.x * m_iBasisNormal + m_halfDimensions.y * rightBasis;
	out_fourCornerWorldPositions[0] = bottomLeft;
	out_fourCornerWorldPositions[1] = bottomRight;
	out_fourCornerWorldPositions[2] = topRight;
	out_fourCornerWorldPositions[3] = topLeft;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OBB2D::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_iBasisNormal.RotateDegrees(rotationDeltaDegrees);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OBB2D::RotateAboutCorner(float rotationDegrees, Vec2 const& rotationPoint)
{
	Vec2 cornerPoints[4];
	GetCornerPoints(cornerPoints);

	for (int i = 0; i < 4; ++i) 
	{
		cornerPoints[i] -= rotationPoint;
	}

	float radians = ConvertDegreesToRadians(rotationDegrees);
	for (int i = 0; i < 4; ++i) 
	{
		cornerPoints[i] = cornerPoints[i].GetRotatedRadians(radians);
	}

	Vec2 newCenter = Vec2(0.f, 0.f);
	for (int i = 0; i < 4; ++i) 
	{
		cornerPoints[i] += rotationPoint;
		newCenter += cornerPoints[i];
	}

	newCenter *= 0.25f; 

	m_center = newCenter;
	m_iBasisNormal.RotateDegrees(rotationDegrees);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 OBB2D::GetLocalPosForWorldPos(Vec2 worldPos) const
{
	Vec2 displacement = worldPos - m_center;
	return Vec2(DotProduct2D(displacement, m_iBasisNormal), DotProduct2D(displacement, m_iBasisNormal.GetRotated90Degrees()));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 OBB2D::GetWorldPosForLocalPos(Vec2 localPos) const
{
	return m_center + (localPos.x * m_iBasisNormal) + (localPos.y * m_iBasisNormal.GetRotated90Degrees());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OBB2D::Translate(Vec2 const& newCenter)
{
	m_center = newCenter;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
