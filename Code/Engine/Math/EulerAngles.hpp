#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Mat44;
struct Vec3;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct EulerAngles
{
public:
	EulerAngles() = default;
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);
	void GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis);
	Mat44 GetAsMatrix_XFwd_YLeft_ZUp() const;
	void SetFromText(char const* text);
	Vec3 GetForwardVector() const;
	Vec3 GetLeftVector() const;
	Vec3 GetUpVector() const;
	void operator+=(const EulerAngles& vecToAdd);
public:
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------