#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct AABB2;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Plane2D
{
	Vec2 m_planeNormal;
	float m_distance;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class ConvexHull2D
{
public:
	bool IsPointInside(Vec2 const& point);
	std::vector<Plane2D> m_boundingPlanes;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class ConvexPoly2D
{
public:
	ConvexPoly2D() = default;
	ConvexPoly2D(std::vector<Vec2> const& ccwPoints);
	
	Vec2 const					GetCenter() const;
	float						GetBoundingDiscRadius(Vec2& discCenter);
	ConvexHull2D				GetConvexHull2D() const;
	std::vector<Vec2> const&	GetPoints() const;
	int							GetTotalPointsSize() const;
	Vec2						GetPointAtIndex(int index) const;
	AABB2						GetBoundingBox() const;
	void						GetMinsAndMaxs(Vec2& boundingMins, Vec2& boundingMaxs);
	
	void						RotateAroundPoint(Vec2 const& refPoint, float deltaAngle);
	void						ScaleAroundPoint(Vec2 const& refPoint, float scale);
	void						Translate(Vec2 const& displacement);
	void						AddPointAndCleanup(Vec2 point);
	bool						CheckIfInsideAABB2(AABB2 bounds);
	bool						CheckIfItOverlapsAABB2(AABB2 bounds);
	void						SetPoints(std::vector<Vec2> const& orderedPoints);
	void						WritePolyToBuffer(std::vector<unsigned char>& buffer) const;

	std::vector<Vec2> m_ccwPoints;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
