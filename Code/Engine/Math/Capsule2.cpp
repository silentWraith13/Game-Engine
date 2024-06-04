#include "Engine/Math/Capsule2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Capsule2::Translate(Vec2 translation)
{
	m_bone.Translate(translation);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Capsule2::SetCenter(Vec2 newCenter)
{
	Vec2 displacement = newCenter - GetCenter();
	m_bone.SetCenter(newCenter);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Capsule2::RotateAboutCenter(float rotationDegrees)
{
	Vec2 center = GetCenter();
	m_bone.m_start = center + (m_bone.m_start - center).GetRotatedDegrees(rotationDegrees);
	m_bone.m_end = center + (m_bone.m_end - center).GetRotatedDegrees(rotationDegrees);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Capsule2::GetCenter() const
{
	return m_bone.GetCenter();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
