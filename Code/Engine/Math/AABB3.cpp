#include "Engine/Math/AABB3.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
 AABB3 AABB3::ZERO_TO_ONE = AABB3(Vec3(0.f, 0.f,0.f), Vec3(1.f, 1.f, 1.f));
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB3::AABB3(AABB3 const& copyFrom)
	:m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	: m_mins(minX, minY, minZ)
	, m_maxs(maxX, maxY, maxZ)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AABB3::Raycast(const Vec3& rayOrigin, const Vec3& rayDir, float out_tNear, float out_tFar) const
{
	float tmin = (m_mins.x - rayOrigin.x) / rayDir.x;
	float tmax = (m_maxs.x - rayOrigin.x) / rayDir.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (m_mins.y - rayOrigin.y) / rayDir.y;
	float tymax = (m_maxs.y - rayOrigin.y) / rayDir.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (m_mins.z - rayOrigin.z) / rayDir.z;
	float tzmax = (m_maxs.z - rayOrigin.z) / rayDir.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	out_tNear = tmin;
	out_tFar = tmax;

	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AABB3::DoesContainAABB3(AABB3 const& bounds)
{
	return (bounds.m_mins.x >= m_mins.x && bounds.m_maxs.x <= m_maxs.x &&
		bounds.m_mins.y >= m_mins.y && bounds.m_maxs.y <= m_maxs.y &&
		bounds.m_mins.z >= m_mins.z && bounds.m_maxs.z <= m_maxs.z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 AABB3::GetSize() const
{
	return m_maxs - m_mins;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const AABB3::GetCenter() const
{
	float x = (m_mins.x + m_maxs.x) * 0.5f;
	float y = (m_mins.y + m_maxs.y) * 0.5f;
	float z = (m_mins.z + m_maxs.z) * 0.5f;
	return Vec3(x, y, z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AABB3::Intersects(AABB3 const& other)
{
	if (m_maxs.x < other.m_mins.x || m_mins.x > other.m_maxs.x) 
	{
		return false;
	}

	if (m_maxs.y < other.m_mins.y || m_mins.y > other.m_maxs.y)
	{
		return false;
	}

	if (m_maxs.z < other.m_mins.z || m_mins.z > other.m_maxs.z)
	{
		return false;
	}

	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
