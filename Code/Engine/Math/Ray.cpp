#include "Engine/Math/Ray.hpp"

Ray::Ray(Vec3 startPosition, Vec3 direction)
{
	m_startPosition = startPosition;
	m_normalizedDirection = direction;
}
