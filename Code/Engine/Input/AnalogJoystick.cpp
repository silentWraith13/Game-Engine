#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float AnalogJoystick::GetMagnitude() const
{
	return m_correctedPosition.GetLength();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float AnalogJoystick::GetOrientationDegrees() const
{
	return m_correctedPosition.GetOrientationDegrees();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2(0.1f, 0.1f);
	m_correctedPosition = Vec2(0.0f, 0.0f);
	m_innerDeadZoneFraction = 0.0f;
	m_outerDeadZoneFraction = 0.0f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold)
{
	m_innerDeadZoneFraction = normalizedInnerDeadzoneThreshold;
	m_outerDeadZoneFraction = normalizedOuterDeadzoneThreshold;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	m_rawPosition = Vec2(rawNormalizedX, rawNormalizedY);
	float fraction = m_rawPosition.GetLength() / 32768.f;
	fraction = GetClamped(fraction, m_innerDeadZoneFraction, m_outerDeadZoneFraction);
	fraction = RangeMap(fraction, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.0f, 1.0f);

	Vec2 correctPosition = m_rawPosition;
	correctPosition.SetLength(fraction);
	m_correctedPosition = correctPosition;
}
