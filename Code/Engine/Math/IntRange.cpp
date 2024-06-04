#include "Engine/Math/IntRange.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntRange::IntRange(int min, int max)
	:m_min(min), m_max(max)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntRange::IsOnRange(int value) const
{
	return value >= m_min && value <= m_max;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntRange::IsOverlappingWith(const IntRange& other) const
{
	return m_min <= other.m_max && m_max >= other.m_min;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntRange::operator!=(const IntRange& other) const
{
	return !(*this == other);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntRange::operator==(const IntRange& other) const
{
	return m_min == other.m_min && m_max == other.m_max;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntRange::operator=(const IntRange& other)
{
	m_min = other.m_min;
	m_max = other.m_max;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntRange IntRange::ZERO(0, 0);
const IntRange IntRange::ONE(1, 1);
const IntRange IntRange::ZERO_TO_ONE(0, 1);