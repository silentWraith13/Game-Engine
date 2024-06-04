#pragma once
#include "Engine/Math/Vec3.hpp"


struct AABB3 
{
public:
	Vec3   m_mins;
	Vec3   m_maxs;

public:

	~AABB3() {}
	AABB3() {}
	AABB3(AABB3 const& copyFrom);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(Vec3 const& mins, Vec3 const& maxs);
	bool Raycast(const Vec3& rayOrigin, const Vec3& rayDir, float out_tNear, float out_tFar) const;
	bool DoesContainAABB3(AABB3 const& bounds);
	Vec3 GetSize() const;
	Vec3 const GetCenter() const;
	bool Intersects(AABB3 const& other);

	static AABB3 ZERO_TO_ONE;
};