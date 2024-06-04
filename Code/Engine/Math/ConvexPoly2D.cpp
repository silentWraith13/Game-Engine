#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/BufferUtils.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ConvexPoly2D::ConvexPoly2D(std::vector<Vec2> const& ccwPoints)
{
	m_ccwPoints.insert(m_ccwPoints.begin(), ccwPoints.begin(), ccwPoints.end());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const ConvexPoly2D::GetCenter() const
{
	Vec2 middlePoint = Vec2(0.f, 0.f);
	for (int vertexIndex = 0; vertexIndex < m_ccwPoints.size(); vertexIndex++) 
	{
		Vec2 const& vertex = m_ccwPoints[vertexIndex];
		middlePoint += vertex;
	}

	middlePoint /= (float)m_ccwPoints.size();
	return middlePoint;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float ConvexPoly2D::GetBoundingDiscRadius(Vec2& discCenter)
{
	discCenter = GetCenter();
	float greatestDistToCenter = FLT_MIN;
	for (Vec2 const& point : m_ccwPoints) 
	{
		float distToCenter = GetDistance2D(point, discCenter);
		if (distToCenter > greatestDistToCenter) 
		{
			greatestDistToCenter = distToCenter;
		}
	}

	return greatestDistToCenter;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ConvexHull2D ConvexPoly2D::GetConvexHull2D() const
{
	ConvexHull2D hull;
	for (int i = 0; i < m_ccwPoints.size() - 1; i++)
	{
		Plane2D plane;

		Vec2 direction = (m_ccwPoints[i] - m_ccwPoints[i + 1]).GetNormalized();
		plane.m_planeNormal = direction.GetRotated90Degrees();
		plane.m_distance = DotProduct2D(m_ccwPoints[i], plane.m_planeNormal);
		hull.m_boundingPlanes.push_back(plane);
	}
	return hull;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::RotateAroundPoint(Vec2 const& refPoint, float deltaAngle) 
{

	for (Vec2& point : m_ccwPoints) {
		Vec2 dispToPoint = point - refPoint;
		dispToPoint.RotateDegrees(deltaAngle);
		point = refPoint + dispToPoint;
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::ScaleAroundPoint(Vec2 const& refPoint, float scale)
{
	for (Vec2& point : m_ccwPoints) 
	{
		Vec2 dispToPoint = point - refPoint;
		dispToPoint *= scale;
		point = refPoint + dispToPoint;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::Translate(Vec2 const& displacement)
{
	for (Vec2& point : m_ccwPoints) 
	{
		point += displacement;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 ConvexPoly2D::GetBoundingBox() const
{
	Vec2 mins = Vec2(FLT_MAX, FLT_MAX);
	Vec2 maxs = Vec2(FLT_MIN, FLT_MIN);

	for (Vec2 const& point : m_ccwPoints) 
	{
		if (point.x < mins.x) 
		{
			mins.x = point.x;
		}

		if (point.y < mins.y) 
		{
			mins.y = point.y;
		}

		if (point.x > maxs.x)
		{
			maxs.x = point.x;
		}

		if (point.y > maxs.y) 
		{
			maxs.y = point.y;
		}
	}
	return AABB2(mins, maxs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::AddPointAndCleanup(Vec2 point)
{
	m_ccwPoints.push_back(point);

	//TO DO :: MAKE SURE POINTS INSIDE ARE CLEANED UP
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int ConvexPoly2D::GetTotalPointsSize() const
{
	return (int)m_ccwPoints.size();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 ConvexPoly2D::GetPointAtIndex(int index) const
{
	return m_ccwPoints[index];
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::vector<Vec2> const& ConvexPoly2D::GetPoints() const
{
	return m_ccwPoints;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::SetPoints(std::vector<Vec2> const& orderedPoints)
{
	m_ccwPoints.clear();
	m_ccwPoints = orderedPoints;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::GetMinsAndMaxs(Vec2& boundingMins, Vec2& boundingMaxs)
{
	for (int i = 0; i < m_ccwPoints.size() - 1; i++)
	{
		if (m_ccwPoints[i].x < boundingMins.x)
		{
			boundingMins.x = m_ccwPoints[i].x;
		}
		if (m_ccwPoints[i].y < boundingMins.y)
		{
			boundingMins.y = m_ccwPoints[i].y;
		}
		if (m_ccwPoints[i].x > boundingMaxs.x)
		{
			boundingMaxs.x = m_ccwPoints[i].x;
		}
		if (m_ccwPoints[i].y > boundingMaxs.y)
		{
			boundingMaxs.y = m_ccwPoints[i].y;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ConvexPoly2D::CheckIfItOverlapsAABB2(AABB2 bounds)
{
	Vec2 mins = bounds.m_mins;
	Vec2 maxs = bounds.m_maxs;

	for (int i = 0; i < m_ccwPoints.size() - 1; i++)
	{
		if (m_ccwPoints[i].x >= mins.x && m_ccwPoints[i].y >= mins.y && m_ccwPoints[i].x <= maxs.x
			&& m_ccwPoints[i].y <= maxs.y)
		{
			return true;
		}
	}
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ConvexPoly2D::CheckIfInsideAABB2(AABB2 bounds)
{
	Vec2 mins = bounds.m_mins;
	Vec2 maxs = bounds.m_maxs;

	for (int i = 0; i < m_ccwPoints.size() - 1; i++)
	{
		if (m_ccwPoints[i].x < mins.x)
		{
			return false;
		}
		if (m_ccwPoints[i].y < mins.y)
		{
			return false;
		}
		if (m_ccwPoints[i].x > maxs.x)
		{
			return false;
		}
		if (m_ccwPoints[i].y > maxs.y)
		{
			return false;
		}
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ConvexHull2D::IsPointInside(Vec2 const& point)
{
	for (int i = 0; i < m_boundingPlanes.size(); i++)
	{
		float dotValue = DotProduct2D(point, m_boundingPlanes[i].m_planeNormal);
		if (dotValue > m_boundingPlanes[i].m_distance)
		{
			return false;
		}
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ConvexPoly2D::WritePolyToBuffer(std::vector<unsigned char>& buffer) const
{
	BufferWriter bufWriter(buffer);

	bufWriter.AppendByte((unsigned char)m_ccwPoints.size());
	for (Vec2 const& vertex : m_ccwPoints) 
	{
		bufWriter.AppendVec2(vertex);
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------