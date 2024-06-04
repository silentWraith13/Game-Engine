#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/StringUtils.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees)
	:m_yawDegrees(yawDegrees), m_pitchDegrees(pitchDegrees), m_rollDegrees(rollDegrees)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis)
{
	Mat44 eulerAsMatrix;
	eulerAsMatrix.AppendZRotation(m_yawDegrees);
	eulerAsMatrix.AppendYRotation(m_pitchDegrees);
	eulerAsMatrix.AppendXRotation(m_rollDegrees);
	out_forwardIBasis = eulerAsMatrix.GetIBasis3D();
	out_leftJBasis = eulerAsMatrix.GetJBasis3D();
	out_upKBasis = eulerAsMatrix.GetKBasis3D();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	Mat44 eulerAsMatrix;
	eulerAsMatrix.AppendZRotation(m_yawDegrees);
	eulerAsMatrix.AppendYRotation(m_pitchDegrees);
	eulerAsMatrix.AppendXRotation(m_rollDegrees);
	return eulerAsMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EulerAngles::SetFromText(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, ',');
	m_yawDegrees = static_cast<float>(atof((string[0].c_str())));
	m_pitchDegrees = static_cast<float>(atof((string[1].c_str())));
	m_rollDegrees = static_cast<float>(atof((string[2].c_str())));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetForwardVector() const
{
	return this->GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D().GetNormalized();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetLeftVector() const
{
	return this->GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D().GetNormalized();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetUpVector() const
{
	return this->GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D().GetNormalized();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EulerAngles::operator+=(const EulerAngles& vecToAdd)
{
	m_yawDegrees += vecToAdd.m_yawDegrees;
	m_pitchDegrees += vecToAdd.m_pitchDegrees;
	m_rollDegrees += vecToAdd.m_rollDegrees;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
