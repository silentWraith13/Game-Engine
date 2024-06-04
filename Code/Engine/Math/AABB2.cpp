#include "Engine/Math/AABB2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 AABB2::ZERO_TO_ONE = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f));
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2::AABB2(AABB2 const& copyFrom)
    :m_mins(copyFrom.m_mins)
    ,m_maxs(copyFrom.m_maxs)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2::AABB2(float minX, float minY, float maxX, float maxY)
    :m_mins(minX,minY)
    ,m_maxs(maxX,maxY)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
    :m_mins(mins)
    ,m_maxs(maxs)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AABB2::operator==(const AABB2& compare) const
{
    return (m_mins==compare.m_mins && m_maxs==compare.m_maxs);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AABB2::IsPointInside(Vec2 const& point) const
{
	if ((point.x > m_mins.x && point.x < m_maxs.x) && (point.y > m_mins.y && point.y < m_maxs.y))
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetCenter() const
{
    float x = (m_mins.x + m_maxs.x) * 0.5f;
    float y = (m_mins.y + m_maxs.y) * 0.5f;
    
    return Vec2(x, y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetDimensions() const
{
    float x = m_maxs.x - m_mins.x;
    float y = m_maxs.y - m_mins.y;
    
    return Vec2(x, y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
    float nearestX = GetClamped(referencePosition.x, m_mins.x, m_maxs.x);
    float nearestY = GetClamped(referencePosition.y, m_mins.y, m_maxs.y);
    return Vec2(nearestX, nearestY);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
    float newX = RangeMap(uv.x,0,1,m_mins.x,m_maxs.x);
    float newY = RangeMap(uv.y, 0, 1, m_mins.y, m_maxs.y);
   
    return Vec2(newX, newY);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
    float newX = RangeMap(point.x, m_mins.x, m_maxs.x, 0, 1);
    float newY= RangeMap(point.y, m_mins.y, m_maxs.y, 0, 1);
    
    return Vec2(newX, newY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxAtUV(const Vec2& uv1, const Vec2& uv2) const
{
	Vec2 point1 = GetPointAtUV(uv1);
	Vec2 point2 = GetPointAtUV(uv2);
	return AABB2(point1, point2);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AABB2::Translate(Vec2 const& translationToApply)
{
    m_mins.x += translationToApply.x;
    m_mins.y += translationToApply.y;
    m_maxs.x += translationToApply.x;
    m_maxs.y += translationToApply.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

void AABB2::SetCenter(Vec2 const& newCenter)
{
    Vec2 center = GetCenter();
    center = newCenter - center;
    
    Translate(center);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
    Vec2 dimensions =(newDimensions - GetDimensions()) * 0.5f;
    m_maxs += dimensions;
    m_mins -= dimensions;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint(Vec2 const& point)
{
    if (point.x > m_maxs.x)
    {
        m_maxs.x = point.x;
    }
    
    if (point.y > m_maxs.y)
    {
        m_maxs.y = point.y;
    }
    
    if (point.x < m_mins.x)
    {
        m_mins.x = point.x;
    }
    
    if (point.y < m_mins.y)
    {
        m_mins.y = point.y;
    }  
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AABB2::AddPadding(float paddingX, float paddingY)
{
	m_mins.x -= paddingX;
	m_mins.y -= paddingY;
	m_maxs.x += paddingX;
	m_maxs.y += paddingY;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxWithin(Vec2 const& point1, Vec2 const& point2) const
{
	Vec2 newMins = Vec2(m_mins.x + point1.x * GetDimensions().x, m_mins.y + point1.y * GetDimensions().y);
	Vec2 newMaxs = Vec2(m_mins.x + point2.x * GetDimensions().x, m_mins.y + point2.y * GetDimensions().y);
	return AABB2(newMins, newMaxs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

