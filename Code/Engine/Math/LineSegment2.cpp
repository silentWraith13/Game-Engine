#include "Engine/Math/LineSegment2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
LineSegment2::LineSegment2()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
LineSegment2::LineSegment2(Vec2 const& boneStart, Vec2 const& boneEnd)
{
	m_start = boneStart;
	m_end = boneEnd;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
LineSegment2::~LineSegment2()
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void LineSegment2::Translate(Vec2 translation)
{
	m_start += translation;
	m_end += translation;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void LineSegment2::SetCenter(Vec2 newCenter)
{
	Vec2 displacement = newCenter - GetCenter();
	m_start += displacement;
	m_end += displacement;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void LineSegment2::RotateAboutCenter(float rotationDegrees)
{
	Vec2 center = GetCenter();
	Vec2 startDisplacement = m_start - center;
	Vec2 endDisplacement = m_end - center;
	m_start = center + startDisplacement.GetRotatedDegrees(rotationDegrees);
	m_end = center + endDisplacement.GetRotatedDegrees(rotationDegrees);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 LineSegment2::GetCenter() const
{
	return (m_start + m_end) * 0.5f;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
