#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/StringUtils.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const FloatRange FloatRange::ZERO(0.0f, 0.0f);
const FloatRange FloatRange::ONE(1.0f, 1.0f);
const FloatRange FloatRange::ZERO_TO_ONE(0.0f, 1.0f);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
FloatRange::FloatRange(float min, float max)
	:m_min(min), m_max(max)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool FloatRange::operator!=(const FloatRange& other) const
{
	return !(*this == other);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool FloatRange::IsOnRange(float value) const
{
	return value >= m_min && value <= m_max;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool FloatRange::IsOverlappingWith(const FloatRange& other) const
{
	return m_min <= other.m_max && m_max >= other.m_min;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void FloatRange::SetFromText(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, '~');
	m_min = static_cast<float>(atof((string[0].c_str())));
	m_max = static_cast<float>(atof((string[1].c_str())));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void FloatRange::operator=(const FloatRange& other) 
{
	m_min = other.m_min;
	m_max = other.m_max;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool FloatRange::operator==(const FloatRange& other) const
{
	return m_min == other.m_min && m_max == other.m_max;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
FloatRange FloatRange::GetOverlappingRange(FloatRange const& compareTo) const
{
	if (!IsOverlappingWith(compareTo)) return FloatRange();

	float biggerMin = 0;
	float smallerMax = 0;
	biggerMin = (compareTo.m_min > m_min) ? compareTo.m_min : m_min;
	smallerMax = (compareTo.m_max < m_max) ? compareTo.m_max : m_max;
	return FloatRange(biggerMin, smallerMax);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------