#include "MathUtils.hpp"
#include <math.h>
#include "Engine/Math/IntVec2.hpp"
#define _USE_MATH_DEFINES
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include <math.h>
#include <float.h>
#include <algorithm>  
#include <cmath>      

//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr float PI = 3.1415926535897932384626433832795f;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float ConvertDegreesToRadians(float degrees)
{	
	float m_radianValue = degrees * (PI / 180.f);
	
	return m_radianValue;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float ConvertRadiansToDegrees(float radians)
{
	float m_degreeValue = radians * (180.0f/PI);
	
	return m_degreeValue;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float CosDegrees(float degrees)
{
	float m_angleRadian = degrees * (PI / 180.f);
	
	return cosf(m_angleRadian);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SinDegrees(float degrees)
{
	float m_angleRadian = degrees * (PI / 180.f);
	
	return sinf(m_angleRadian);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Atan2Degrees(float y, float x)			
{
	float m_thetaRadians = atan2f(y, x);
	float thetaDegrees = m_thetaRadians * (180.0f / PI);
	
	return thetaDegrees;	
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	float angularDisplacement = endDegrees - startDegrees;
	
	while (angularDisplacement > 180.0f)
	{
		 angularDisplacement -= 360.0f;
	}
	
	while (angularDisplacement < -180.0f)
	{
		 angularDisplacement += 360.0f;
	}
	
	return angularDisplacement;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float angle = GetShortestAngularDispDegrees(currentDegrees,goalDegrees);
	
	if (fabsf(angle) <= maxDeltaDegrees)
	{
		return goalDegrees;
	}
	
	if (angle > 0)
	{
		return currentDegrees + maxDeltaDegrees;
	}
	
	else
	{
		return currentDegrees - maxDeltaDegrees;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	float aMag = a.GetLength();
	float bMag = b.GetLength();
	float dotProductBetweenTwoVectors = DotProduct2D(a, b);
	float multiMag = aMag * bMag;
	float d = dotProductBetweenTwoVectors / multiMag;
	float theta = acosf(d);
	float thetaDegrees=ConvertRadiansToDegrees(theta);
	return thetaDegrees;	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Sigmoid(float x)
{
	return 1.0f / (1.0f + exp(-x));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	
	if (value > maxValue)
	{
		return maxValue;
	}	
	else 
	{
		return value;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int GetClamped(int value, int minValue, int maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}

	if (value > maxValue)
	{
		return maxValue;
	}
	else
	{
		return value;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetClampedZeroToOne(float value)
{
	return GetClamped(value, 0.f, 1.f);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Interpolate(float start, float end, float fractionTowardsEnd)
{	
	return (start + (fractionTowardsEnd) * (end - start));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 Interpolate(Rgba8 start, Rgba8 end, float factor)
{
	Rgba8 resultColor;

	resultColor.r = static_cast<unsigned char>(start.r + factor * (end.r - start.r));
	resultColor.g = static_cast<unsigned char>(start.g + factor * (end.g - start.g));
	resultColor.b = static_cast<unsigned char>(start.b + factor * (end.b - start.b));
	resultColor.a = static_cast<unsigned char>(start.a + factor * (end.a - start.a));

	return resultColor;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
EulerAngles InterpolateEulerAngles(EulerAngles const& start, EulerAngles const& end, float factor)
{
	EulerAngles result;
	result.m_yawDegrees = Interpolate(start.m_yawDegrees, end.m_yawDegrees, factor);
	result.m_pitchDegrees = Interpolate(start.m_pitchDegrees, end.m_pitchDegrees, factor);
	result.m_rollDegrees = Interpolate(start.m_rollDegrees, end.m_rollDegrees, factor);

	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int Int_Interpolate(int start, int end, int fractionTowardsEnd)
{
	return (start + (fractionTowardsEnd) * (end - start));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 ScaleVec3(Vec3 const& value, float posX, float posY, float posZ, float negX, float negY, float negZ)
{
	Vec3 result = value;
	
	if (result.x > 0)
	{
		result.x *= posX;
	}
	else if (result.x < 0)
	{
		result.x *= negX;
	}

	if (result.y > 0)
	{
		result.y *= posY;
	}
	else if (result.y < 0)
	{
		result.y *= negY;
	}

	if (result.z > 0)
	{
		result.z *= posZ;
	}
	else if (result.z < 0)
	{
		result.z *= negZ;
	}

	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 GetClampedVec3(Vec3 const& value, Vec3 const& minValue, Vec3 const& maxValue)
{
	Vec3 clamped;

	clamped.x = GetClamped(value.x, minValue.x, maxValue.x);
	clamped.y = GetClamped(value.y, minValue.y, maxValue.y);
	clamped.z = GetClamped(value.z, minValue.z, maxValue.z);

	return clamped;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 HueToRgba8(float hue)
{
	hue = fmodf(hue, 1.0f);
	if (hue < 0.0f) hue += 1.0f;

	float r = 0.0f, g = 0.0f, b = 0.0f;

	// Calculate the RGB values based on hue
	// The hue is divided into six sectors.
	if (hue < 1.0f / 6.0f)
	{
		r = 1.0f;
		g = hue * 6.0f;
	}
	else if (hue < 2.0f / 6.0f)
	{
		r = (2.0f / 6.0f - hue) * 6.0f;
		g = 1.0f;
	}
	else if (hue < 3.0f / 6.0f)
	{
		g = 1.0f;
		b = (hue - 2.0f / 6.0f) * 6.0f;
	}
	else if (hue < 4.0f / 6.0f)
	{
		g = (4.0f / 6.0f - hue) * 6.0f;
		b = 1.0f;
	}
	else if (hue < 5.0f / 6.0f)
	{
		r = (hue - 4.0f / 6.0f) * 6.0f;
		b = 1.0f;
	}
	else
	{
		r = 1.0f;
		b = (6.0f / 6.0f - hue) * 6.0f;
	}

	// Convert to 0-255 range and create the Rgba8 color
	unsigned char red = static_cast<unsigned char>(r * 255.0f);
	unsigned char green = static_cast<unsigned char>(g * 255.0f);
	unsigned char blue = static_cast<unsigned char>(b * 255.0f);

	return Rgba8(red, green, blue);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 InterpolateVec2(const Vec2& start, const Vec2& end, float fractionTowardsEnd)
{
	Vec2 result;
	result.x = Interpolate(start.x, end.x, fractionTowardsEnd);
	result.y = Interpolate(start.y, end.y, fractionTowardsEnd);
	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	if (rangeStart == rangeEnd)
	{
		return .50f;
	}
	
	else
	{
		return (value - rangeStart) / (rangeEnd - rangeStart);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float fractionVal= GetFractionWithinRange(inValue, inStart, inEnd);
	
	return Interpolate(outStart,outEnd,fractionVal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float fraction = GetFractionWithinRange(inValue, inStart, inEnd);
	fraction = GetClampedZeroToOne(fraction);
	
	return Interpolate(outStart, outEnd, fraction);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int RoundDownToInt(float value)
{
	if (value > 0)
	{
		return static_cast<int>(value);
	}
	
	else if (value < 0)
	{
		return static_cast<int>(value - 1);
	}
	
	else
		return static_cast<int>(value);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 GetRandomColor(RandomNumberGenerator rng)
{

	unsigned char r = static_cast<unsigned char>(rng.RollRandomIntInRange(0, 255));
	unsigned char g = static_cast<unsigned char>(rng.RollRandomIntInRange(0, 255));
	unsigned char b = static_cast<unsigned char>(rng.RollRandomIntInRange(0, 255));
	unsigned char a = static_cast<unsigned char>(40);

	return Rgba8{ r, g, b, a };
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.x + a.y * b.y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return ( (a.x * b.y) - (a.y * b.x) );
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	return Vec3( (a.y * b.z - a.z * b.y), ( a.z * b.x - a.x * b.z), ( a.x * b.y - a.y * b.x) );
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float x1 = positionA.x;
	float x2 = positionB.x;
	float y1 = positionA.y;
	float y2 = positionB.y;
	
	float distance = sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	return distance;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	float distanceSquared = ((dx * dx) + (dy * dy));
	return distanceSquared;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)		
{
	float x1 = positionA.x;
	float x2 = positionB.x;
	float y1 = positionA.y;
	float y2 = positionB.y;
	float z1 = positionA.z;
	float z2 = positionB.z;

	float distance = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)+(z2-z1)* (z2 - z1));
	
	return distance;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)   
{
	float x1 = positionA.x;
	float x2 = positionB.x;
	float y1 = positionA.y;
	float y2 = positionB.y;
	float z1 = positionA.z;
	float z2 = positionB.z;

	float distance = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
	float distanceSquared = distance * distance;
	
	return distanceSquared;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float x1 = positionA.x;
	float x2 = positionB.x;
	float y1 = positionA.y;
	float y2 = positionB.y;
	
	float distance = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	
	return distance;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float x1 = positionA.x;
	float x2 = positionB.x;
	float y1 = positionA.y;
	float y2 = positionB.y;

	float distance = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	float distanceSquared = distance * distance;
	
	return distanceSquared;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetDistanceFromPointToLineSegment2D(Vec2 const& point, LineSegment2 const& lineSegment)
{
	Vec2 segmentVector = lineSegment.m_end - lineSegment.m_start;
	float segmentLength = segmentVector.GetLength();
	Vec2 segmentDirection = segmentVector / segmentLength;

	Vec2 pointToSegmentStart = lineSegment.m_start - point;
	float projectionDistance = DotProduct2D(segmentDirection, pointToSegmentStart);

	if (projectionDistance <= 0.f) {
		return GetDistance2D(point, lineSegment.m_start);
	}
	else if (projectionDistance >= segmentLength) {
		return GetDistance2D(point, lineSegment.m_end);
	}
	else {
		Vec2 projectionPoint = lineSegment.m_start + segmentDirection * projectionDistance;
		return GetDistance2D(point, projectionPoint);
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	IntVec2 temp = IntVec2( pointA.x - pointB.x, pointA.y - pointB.y);
	return temp.GetTaxicabLength();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return DotProduct2D(vectorToProject, vectorToProjectOnto.GetNormalized());
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return (DotProduct2D(vectorToProject,vectorToProjectOnto.GetNormalized()) * vectorToProjectOnto.GetNormalized());
}

Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	return (DotProduct3D(vectorToProject, vectorToProjectOnto.GetNormalized()) * vectorToProjectOnto.GetNormalized());
}

Vec3 const GetPerpendicularComponent3D(Vec3 const& vectorToAdjust, Vec3 const& axis)
{
	Vec3 projection = DotProduct3D(vectorToAdjust, axis.GetNormalized()) * axis.GetNormalized();
	return vectorToAdjust - projection;
}

Vec3 GetProjectedOntoPlane(Vec3 const& vectorToProject, Vec3 const& planeNormal)
{
	Vec3 normal = planeNormal.GetNormalized();
	return vectorToProject - DotProduct3D(vectorToProject, normal) * normal;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float sumOfRadii = radiusA + radiusB;
	float disSquaredBetweenCenters = GetDistanceSquared2D(centerA, centerB);
	
	return disSquaredBetweenCenters < (sumOfRadii* sumOfRadii);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float sumOfRadii = radiusA + radiusB;
	float disSquaredBetweenCenters = GetDistanceSquared3D(centerA, centerB);
	
	return disSquaredBetweenCenters < (sumOfRadii* sumOfRadii);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DoAAB3DsOverlap(AABB3 const& first, AABB3 const& second)
{
	if (first.m_maxs.x < second.m_mins.x)
		return false;

	if (first.m_mins.x > second.m_maxs.x)
		return false;

	if (first.m_maxs.y < second.m_mins.y)
		return false;

	if (first.m_mins.y > second.m_maxs.y)
		return false;

	if (first.m_maxs.z < second.m_mins.z)
		return false;

	if (first.m_mins.z > second.m_maxs.z)
		return false;

	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	float distBetweenCenterAndPoint = GetDistance2D(discCenter,point);
	if (distBetweenCenterAndPoint < discRadius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius))
	{
		return false;
	}

	Vec2 dispFromSectorTipToPoint = point - sectorTip;
	float pointRelativeOrientationFromSectorTip = dispFromSectorTipToPoint.GetOrientationDegrees();
	float degreesOfCenter = GetShortestAngularDispDegrees(sectorForwardDegrees, pointRelativeOrientationFromSectorTip);
	return fabs(degreesOfCenter) < (sectorApertureDegrees * 0.5f);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius))
	{
		return false;
	}

	Vec2 dispFromSectorTipToPoint = point - sectorTip;
	float degreesOfCenter = GetAngleDegreesBetweenVectors2D(dispFromSectorTipToPoint, sectorForwardNormal);
	return degreesOfCenter < (sectorApertureDegrees * 0.5f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideDirectedCone3D(const Vec3& point, const Vec3& coneTip, const Vec3& coneForwardNormal, float coneApertureDegrees, float coneRadius)
{
	Vec3 dispFromConeTipToPoint = point - coneTip;
	float distanceToPoint = dispFromConeTipToPoint.GetLength();

	if (distanceToPoint > coneRadius)
	{
		return false;
	}

	Vec3 dispFromConeTipToPointNormalized = dispFromConeTipToPoint.GetNormalized();
	float dotProduct = DotProduct3D(coneForwardNormal, dispFromConeTipToPointNormalized);
	float angleBetweenRadians = acosf(dotProduct);
	float angleBetweenDegrees = ConvertRadiansToDegrees(angleBetweenRadians); 

	return angleBetweenDegrees < (coneApertureDegrees * 0.5f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideOBB2D(Vec2 const& point, OBB2D const& orientedBox)
{
	// 1. Translate point and OBB2D to origin-centered coordinates
	Vec2 centeredPoint = point - orientedBox.m_center;
	
	// 2. Rotate point and OBBD by negative angle
	float negAngleRadians = -atan2f(orientedBox.m_iBasisNormal.y, orientedBox.m_iBasisNormal.x);
	Vec2 rotatedPoint = centeredPoint.GetRotatedRadians(negAngleRadians);
	Vec2 rotatedI = orientedBox.m_iBasisNormal.GetRotated90Degrees();

	// 3. Check if rotated point is within half-dimensions along each local axis
	bool xInRange = fabsf(rotatedPoint.x) <= orientedBox.m_halfDimensions.x;
	bool yInRange = fabsf(rotatedPoint.y) <= orientedBox.m_halfDimensions.y;

	// 4. Return true if point is inside OBB2D
	return xInRange && yInRange;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	// Calculate the nearest point on the line segment between boneStart and boneEnd
	Vec2 const nearestPoint = GetNearestPointOnLineSegment2D(point, boneStart, boneEnd);

	// Check if the distance between the nearest point and the original point is less than the radius
	float const distSquared = GetDistanceSquared2D(point, nearestPoint);
	return (distSquared <= (radius * radius));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule, float radius)
{
	Vec2 boneStart = capsule.m_bone.m_start;
	Vec2 boneEnd = capsule.m_bone.m_end;
	return IsPointInsideCapsule2D(point, boneStart, boneEnd, radius);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	return box.IsPointInside(point);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideLineSegment2D(Vec2 const& point, LineSegment2 const& lineSegment)
{
	Vec2 v1 = lineSegment.m_start - point;
	Vec2 v2 = lineSegment.m_end - point;

	// Check if the point is within the line segment bounds
	if (DotProduct2D(v1, v2) > 0) 
	{
		return false;
	}

	float dist = GetDistanceFromPointToLineSegment2D(point, lineSegment);
	return dist <= GetDistance2D(lineSegment.m_start, lineSegment.m_end) / 2.0f;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideCapsule3D(Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius)
{
	Vec3 SE = boneEnd - boneStart;
	Vec3 SP = point - boneStart;
	if (DotProduct3D(SE, SP) < 0.f)
	{
		//Voronoi region I, closest point on bone to P is start
		return GetDistanceSquared3D(point, boneStart) < radius * radius;
	}

	Vec3 EP = point - boneEnd;
	if(DotProduct3D(SE, EP) > 0.f)
	{
		//Voronoi region II, closest point on bone to P is end
		return GetDistanceSquared3D(point, boneEnd) < radius * radius;
	}
	
	//Voronoi region III, closest point is along bone somewhere in the middle
	Vec3 SN = GetProjectedOnto3D(SP, SE); // displacement from Start to nearest point
	Vec3 N =  boneStart + SN;
	return GetDistanceSquared3D(point, N) < radius * radius;
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideHexagon2D(const Vec2& P, const Vec2& center, float radius)
{
	constexpr int NUM_SIDES = 6;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;

	Vec2 previousVertex = center + Vec2::MakeFromPolarDegrees(0, radius); // Starting with the first vertex

	for (int sideNum = 1; sideNum <= NUM_SIDES; ++sideNum) // <= because we want to loop back to the first vertex
	{
		float degrees = DEGREES_PER_SIDE * (float)sideNum;
		Vec2 currentVertex = center + Vec2::MakeFromPolarDegrees(degrees, radius);

		// Calculate edge vector to next vertex
		Vec2 E = currentVertex - previousVertex;

		// Calculate vector to point P
		Vec2 T = P - previousVertex;

		// Calculate cross product of the two vectors
		float crossZ = CrossProduct2D(E, T);

		// If cross product z-component is negative for any vertex, point is outside
		if (crossZ < 0)
			return false;

		previousVertex = currentVertex; // Move to the next vertex
	}

	// If cross product z-component is positive for all vertexes, point is inside
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideConvexPoly2D(Vec2 const& refPoint, ConvexPoly2D const& convexPoly, float tolerance)
{
	ConvexHull2D asConvexHull = convexPoly.GetConvexHull2D();
	return IsPointInsideConvexHull2D(refPoint, asConvexHull, tolerance);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideConvexHull2D(Vec2 const& refPoint, ConvexHull2D const& convexHull, float tolerance)
{
	for (Plane2D const& plane : convexHull.m_boundingPlanes) 
	{
		float dotProd = DotProduct2D(plane.m_planeNormal, refPoint) - plane.m_distance;

		if (dotProd > tolerance) return false;
	}

	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointInCapsule2D(Vec2 const& referencePosition, Vec2& boneStart, Vec2& boneEnd, float radius)
{
	// Find the nearest point on the line segment formed by the bone of the capsule
	LineSegment2 bone(boneStart, boneEnd);
	Vec2 nearestPointOnBone = GetNearestPointOnLineSegment2D(referencePosition, bone);

	// Clamp the nearest point to the capsule's radius
	Vec2 displacementFromNearestToRef = referencePosition - nearestPointOnBone;
	float displacementLength = displacementFromNearestToRef.GetLength();
	if (displacementLength <= radius) 
	{
		return nearestPointOnBone; // the reference point is already inside the capsule
	}
	else 
	{
		Vec2 clampedDisplacement = displacementFromNearestToRef.GetClamped(radius);
		Vec2 nearestPointInCapsule = nearestPointOnBone + clampedDisplacement;
		return nearestPointInCapsule;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointInCapsule2D(Vec2 const& referencePosition, Capsule2& capsule)
{
	// Find the nearest point on the line segment formed by the bone of the capsule
	Vec2 boneStart = capsule.m_bone.m_start;
	Vec2 boneEnd = capsule.m_bone.m_end;
	LineSegment2 bone(boneStart, boneEnd);
	Vec2 nearestPointOnBone = GetNearestPointOnLineSegment2D(referencePosition, bone);
	// Clamp the nearest point to the capsule's radius
	Vec2 displacementFromNearestToRef = referencePosition - nearestPointOnBone;
	float displacementLength = displacementFromNearestToRef.GetLength();
	if (displacementLength <= capsule.radius)
	{
		return nearestPointOnBone; // the reference point is already inside the capsule
	}
	else
	{
		Vec2 clampedDisplacement = displacementFromNearestToRef.GetNormalized() * capsule.radius;
		Vec2 nearestPointInCapsule = nearestPointOnBone + clampedDisplacement;
		return nearestPointInCapsule;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	Vec2 diff = referencePosition - discCenter;
	diff.ClampLength(discRadius);
	Vec2 nearestPoint = discCenter + diff;
	return nearestPoint;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestpointOnOBB2D(Vec2 const& referencePosition, OBB2D const& orientedBox)
{
	// Step 1: Get the position of the point in the local space of the OBB.
	Vec2 localPos = orientedBox.GetLocalPosForWorldPos(referencePosition);

	// Step 2: Clamp the local position of the point to the dimensions of the OBB to get the closest point in the OBB's local space.
	Vec2 clampedLocalPos = Vec2(GetClamped(localPos.x, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x),
		GetClamped(localPos.y, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y));

	// Step 3: Transform the local position back into world space using the OBB's orientation and center.
	Vec2 worldPos = orientedBox.GetWorldPosForLocalPos(clampedLocalPos);

	return worldPos;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd)
{
	Vec2 SE = lineSegEnd - lineSegStart;
	Vec2 EP = referencePos - lineSegEnd;
	if (DotProduct2D(SE, EP) >= 0.f)
	{
		return lineSegEnd;
	}
	Vec2 SP = referencePos - lineSegStart;
	if (DotProduct2D(SE, EP) <= 0.f)
	{
		return lineSegStart;
	}
	Vec2 SN = GetProjectedOnto2D(SP, SE);
	return lineSegStart + SN;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& lineSegment)
{
	Vec2 SE = lineSegment.m_end - lineSegment.m_start;
	Vec2 EP = referencePos - lineSegment.m_end;
	Vec2 SP = referencePos - lineSegment.m_start;
	if (DotProduct2D(SP, SE) <= 0.f)
	{
		return lineSegment.m_start;
	}
	if (DotProduct2D(EP, SE) >= 0.f)
	{
		return lineSegment.m_end;
	}
	Vec2 SN = GetProjectedOnto2D(SP, SE);
	return lineSegment.m_start + SN;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 const lineDir =Vec2(anotherPointOnLine.x - pointOnLine.x, anotherPointOnLine.y - pointOnLine.y);
	Vec2 const refToStart = Vec2(referencePosition.x - pointOnLine.x, referencePosition.y - pointOnLine.y);
	float const projection = (refToStart.x * lineDir.x + refToStart.y * lineDir.y) / (lineDir.x * lineDir.x + lineDir.y * lineDir.y);
	Vec2 const nearestPoint =Vec2(pointOnLine.x + projection * lineDir.x, pointOnLine.y + projection * lineDir.y);
	return nearestPoint;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, LineSegment2 const& infiniteLine)
{
	Vec2 const lineDir = Vec2(infiniteLine.m_end.x - infiniteLine.m_start.x, infiniteLine.m_end.y - infiniteLine.m_start.y);
	Vec2 const refToStart = Vec2(referencePosition.x - infiniteLine.m_start.x, referencePosition.y - infiniteLine.m_start.y);
	float const projection = (refToStart.x * lineDir.x + refToStart.y * lineDir.y) / (lineDir.x * lineDir.x + lineDir.y * lineDir.y);
	Vec2 const nearestPoint = Vec2(infiniteLine.m_start.x + projection * lineDir.x, infiniteLine.m_start.y + projection * lineDir.y);
	return nearestPoint;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2& box)
{
	return box.GetNearestPoint(referencePos);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	float disTop = GetDistance2D(mobileDiscCenter, fixedPoint);
	Vec2 d = mobileDiscCenter - fixedPoint;
	float overlapDistance = discRadius - disTop;

	if (disTop > discRadius)
	{
		return false;
	}
	
	else
	{
		d.SetLength(overlapDistance);
		mobileDiscCenter += d;
		return true;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	float distanceBetweenCenters = GetDistance2D(mobileDiscCenter,fixedDiscCenter);
	Vec2 d = mobileDiscCenter - fixedDiscCenter;
	float overlapDistance = mobileDiscRadius + fixedDiscRadius - (distanceBetweenCenters);

	if (distanceBetweenCenters > (mobileDiscRadius + fixedDiscRadius))
	{
		return false;
	}
	
	else
	{
		d.SetLength(overlapDistance);
		mobileDiscCenter += d;
		return true;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	float distanceBetweenCenters = GetDistance2D(aCenter, bCenter);
	Vec2 d = aCenter - bCenter;
	float overlapDistance = aRadius + bRadius - (distanceBetweenCenters);

	if (distanceBetweenCenters > (aRadius + bRadius))
	{
		return false;
	}
	else
	{
		d.SetLength(overlapDistance * 0.5f);
		aCenter += d;
		bCenter -= d;
		return true;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 nearestPointOnBoxNearCircleCenter = fixedBox.GetNearestPoint(mobileDiscCenter);
	return PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPointOnBoxNearCircleCenter);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BounceDiscOffFixedDisc2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, Vec2 const& fixedCenter, float fixedRadius, float elasticity)
{

	if (!DoDiscsOverlap(mobileCenter, mobileRadius, fixedCenter, fixedRadius))
		return;

	Vec2 normal = (fixedCenter - mobileCenter).GetNormalized();
	float dotProduct = DotProduct2D(mobileVelocity, normal);
	Vec2 normalComponent = normal * dotProduct;
	Vec2 tangentialComponent = mobileVelocity - normalComponent;

	mobileVelocity = tangentialComponent - (normalComponent * elasticity);

	PushDiscOutOfFixedDisc2D(mobileCenter, mobileRadius, fixedCenter, fixedRadius);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BounceDiscOffDisc2D(Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticity)
{
	if (!DoDiscsOverlap(centerA, radiusA, centerB, radiusB))
		return;

	PushDiscsOutOfEachOther2D(centerA, radiusA, centerB, radiusB);
	
	Vec2 normal = (centerB - centerA).GetNormalized();

	float speedOfAAlongN = DotProduct2D(velocityA, normal);
	Vec2 velocityOfAAlongN = normal * speedOfAAlongN;
	Vec2 velocityOfAAlongT = velocityA - velocityOfAAlongN;

	float speedOfBAlongN = DotProduct2D(velocityB, normal);
	Vec2 velocityOfBAlongN = normal * speedOfBAlongN;
	Vec2 velocityOfBAlongT = velocityB - velocityOfBAlongN;

	// Check if the discs are not converging
	if (speedOfAAlongN - speedOfBAlongN > 0)
	{
		// Swap normal components
		Vec2 temp = velocityOfAAlongN;
		velocityOfAAlongN = velocityOfBAlongN;
		velocityOfBAlongN = temp;

		velocityOfAAlongN *= elasticity;
		velocityOfBAlongN *= elasticity;
	}

	velocityA = velocityOfAAlongT + (velocityOfAAlongN );
	velocityB = velocityOfBAlongT + (velocityOfBAlongN );
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BounceDiscOffOfPoint2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, Vec2 const& fixedCenter, float elasticity)
{
	return BounceDiscOffFixedDisc2D(mobileCenter, mobileRadius, mobileVelocity, fixedCenter, 0.f, elasticity);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BounceOffFixedOBB2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, OBB2D const& box, float elasticity)
{
	Vec2 closestPoint = box.GetLocalPosForWorldPos(mobileCenter);

	// Clamp the closest point to the edges of the OBB
	closestPoint.x = std::max(-box.m_halfDimensions.x, std::min(closestPoint.x, box.m_halfDimensions.x));
	closestPoint.y = std::max(-box.m_halfDimensions.y, std::min(closestPoint.y, box.m_halfDimensions.y));

	// Convert the clamped local point back to world space
	closestPoint = box.GetWorldPosForLocalPos(closestPoint);

	// Check if the mobile object is intersecting with the OBB
	Vec2 displacementFromClosestPoint = mobileCenter - closestPoint;
	float distanceSquared = displacementFromClosestPoint.GetLengthSquared();
	if (distanceSquared >= (mobileRadius * mobileRadius))
		return; // No intersection, exit the function

	// The mobile object is intersecting with the OBB
	float distance = sqrtf(distanceSquared);
	Vec2 normal = (distance > 0.f) ? (displacementFromClosestPoint / distance) : Vec2(0.f, 0.f); // Normalize the displacement vector to get the normal

	// Reflect the mobile object's velocity off the OBB
	float velocityAlongNormal = DotProduct2D(mobileVelocity, normal);
	if (velocityAlongNormal < 0.f)
	{
		// Only apply reflection if the mobile object is moving towards the OBB
		Vec2 reflection = mobileVelocity - 2.f * velocityAlongNormal * normal;
		mobileVelocity = reflection * elasticity;

		// Reposition the mobile object outside the OBB to avoid sinking issues
		float penetrationDepth = mobileRadius - distance;
		mobileCenter += normal * penetrationDepth;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BounceOffFixedAABB2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, AABB2& box, float elasticity)
{
	if (IsPointInsideAABB2D(mobileCenter, box))
		return;

	Vec2 bouncePos = GetNearestPointOnAABB2D(mobileCenter, box);
	BounceDiscOffOfPoint2D(mobileCenter, mobileRadius, mobileVelocity, bouncePos, elasticity);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BounceOffFixedCapsule2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, Capsule2& capsule, float elasticity)
{
	if (IsPointInsideCapsule2D(mobileCenter, capsule, mobileRadius))
		return;

	Vec2 bouncePos = GetNearestPointInCapsule2D(mobileCenter, capsule);
	BounceDiscOffOfPoint2D(mobileCenter, mobileRadius, mobileVelocity, bouncePos, elasticity);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation)  
{
	posToTransform.x *= uniformScale;
	posToTransform.y *= uniformScale;
	
	float thetaDegrees = Atan2Degrees(posToTransform.y, posToTransform.x);
	float length = sqrtf((posToTransform.x * posToTransform.x) + (posToTransform.y * posToTransform.y));
	thetaDegrees += rotationDegrees;

	posToTransform.x = translation.x + (length * CosDegrees(thetaDegrees));
	posToTransform.y = translation.y+ (length * SinDegrees(thetaDegrees));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)  
{
	// 1. Scale
	positionToTransform.x *= scaleXY;
	positionToTransform.y *= scaleXY;

	// 2. Rotate in local space
	float thetaDegrees = Atan2Degrees(positionToTransform.y, positionToTransform.x) + zRotationDegrees;
	float length = sqrtf((positionToTransform.x * positionToTransform.x) + (positionToTransform.y * positionToTransform.y));
	positionToTransform.x = length * CosDegrees(thetaDegrees);
	positionToTransform.y = length * SinDegrees(thetaDegrees);

	// 3. Translate into world space
	positionToTransform.x += translationXY.x;
	positionToTransform.y += translationXY.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float NormalizeByte(unsigned char byteValue)
{
	return (float)byteValue / 255.0f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned char DenormalizeByte(float zeroToOne)
{
	float denormalizedByte = RangeMap(zeroToOne, 0.f, 1.f, 0.f, 256.f);
	if (denormalizedByte > 255.f)
		denormalizedByte = 255.f;
	return static_cast<unsigned char>(denormalizedByte);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& camMatrix, const Vec3& billboardPos, const Vec2& billboardScale /*= Vec2(1.0f, 1.0f)*/)
{
	Vec3 I(1.f, 0.f, 0.f);
	Vec3 J(0.f, 1.f, 0.f);
	Vec3 K(0.f, 0.f, 1.f);

	Vec3 T = billboardPos;

	Vec3 F = camMatrix.GetIBasis3D();
	Vec3 L = camMatrix.GetJBasis3D();
	Vec3 U = camMatrix.GetKBasis3D();
	Vec3 P = camMatrix.GetTranslation3D();

	Vec3 Z(0.f, 0.f, 1.f);
	Vec3 Y(0.f, 1.f, 0.f);

	switch (billboardType)
	{
	case BillboardType::WORLD_UP_CAMERA_FACING:
		K = Z;
		I = P - T;
		I.z = 0.f;
		I.Normalize();
		J = CrossProduct3D(K, I);
		J.Normalize();
		break;

	case BillboardType::FULL_CAMERA_FACING:
		I = P - T;
		I.GetNormalized();
		if (fabsf(DotProduct3D(I, Z) < 1.0f))
		{
			J = CrossProduct3D(Z, I);
			J.GetNormalized();
			K = CrossProduct3D(I, J);
			K.GetNormalized();
		}
		else
		{
			K = CrossProduct3D(I, Y);
			K.GetNormalized();
			J = CrossProduct3D(K, I);
			J.GetNormalized();
		}
		break;

	case BillboardType::WORLD_UP_CAMERA_OPPOSING:
		I = -1 * F;
		I.z = 0.f;
		I.Normalize();
		J = CrossProduct3D(K, I);
		J.Normalize();
		break;

	case BillboardType::FULL_CAMERA_OPPOSING:
		I = -1 * F;
		J = -1 * L;
		K = U;
		break;
	
	case BillboardType::CUSTOM_PROJECTILE:
		K = Z; // Set the K vector to the Z-axis direction
		I = CrossProduct3D(J, K);
		I.Normalize();
		J = CrossProduct3D(K, I);
		J.Normalize();
		break;
	
	default:
		break;
	}
	
	Mat44 billboardMatrix;
	billboardMatrix.SetIJKT3D(I, J, K, T);
	billboardMatrix.AppendScaleNonUniform2D(billboardScale);

	return billboardMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 forwardDir, float maxDist, Vec2 discCenter, float discRadius)
{
	RaycastResult2D result;

	// Step 1: Raycast start (S), Raycast forward normal (i), Raycast maximum length (m), Disc center (C), Disc radius (r)

	Vec2 S = startPos;
	Vec2 i = forwardDir.GetNormalized();
	float m = maxDist;
	Vec2 C = discCenter;
	float r = discRadius;

	// Step 2: Compute ray left normal (j) as (-iy, ix)
	Vec2 j = Vec2(-i.y, i.x);

	// Step 3: Compute displacement SC = C - S
	Vec2 SC = C - S;

	// Step 4: Compute SCj = Dot( SC, j )
	float SCj = DotProduct2D(SC, j);

	// Step 5: If ( SCj > r or SCj < -r ) then return miss – disc is too far to left or right
	if (SCj > r || SCj < -r) 
	{
		return result;
	}

	// Step 6: Compute SCi = Dot( SC, i )
	float SCi = DotProduct2D(SC, i);

	// Step 7: If ( SCi < -r or SCi > m+r ) then miss: disc is entirely before or after ray
	if (SCi < -r || SCi > m + r) 
	{
		return result;
	}

	// Step 8: If S is inside the disc, return hit: impact position = S, distance = 0, impact normal = -i (backward)
	float distToCenter = SC.GetLength();
	if (distToCenter < r) {
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = S;
		result.m_impactNormal = -i;
		return result;
	}

	// Step 9: Compute the distance ‘a’ by solving Pythagoras: a2 + SCj2 = r2, therefore a = sqrt( r2 – SCj2 )
	float a = sqrtf(r * r - SCj * SCj);

	// Step 10: Compute impactDist = SCi – a
	float impactDist = SCi - a;

	// Step 11: If ( impactDist < 0 or impactDist > m ) return miss; impact is before ray start, or after ray end
	if (impactDist < 0.f || impactDist > m) 
	{
		return result;
	}

	// Step 12: else Hit!
	result.m_didImpact = true;

	// Step 13: Compute impactPos = S + (i * impactDist)
	result.m_impactPos = S + i * impactDist;

	// Step 14: Compute impactNormal = (impactPos – C).GetNormalized()
	result.m_impactNormal = (result.m_impactPos - C).GetNormalized();

	// Set impact distance
	result.m_impactDist = impactDist;

	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 forwardDir, float maxDist, Vec2 lineStart, Vec2 lineEnd)
{
	RaycastResult2D result;

	Vec2 S = startPos;
	Vec2 i = forwardDir.GetNormalized();
	float m = maxDist;
	Vec2 A = lineStart;
	Vec2 B = lineEnd;
	Vec2 j = i.GetRotated90Degrees();

	Vec2 SA = S - A;
	Vec2 SB = S - B;
	float SAj = DotProduct2D(SA, j);
	float SBj = DotProduct2D(SB, j);

	if (SAj * SBj >= 0) {
		return result; // Miss (not straddling)
	}

	if (SAj <= 0 && SBj <= 0) {
		return result; // Miss (entirely right of ray)
	}

	if (SAj >= 0 && SBj >= 0) {
		return result; // Miss (entirely left of ray)
	}

	float t = -SAj / (SBj - SAj);
	Vec2 P = A + t * (B - A);
	Vec2 SP = P - S;
	float impactDist = DotProduct2D(SP, i);

	if (impactDist < 0 || impactDist >= m) {
		return result; // Miss (impact distance out of range)
	}

	result.m_didImpact = true;
	result.m_impactDist = impactDist;
	result.m_impactPos = P;
	result.m_rayStartPosition = S;
	result.m_rayDirection = i;
	result.m_rayLength = m;

	Vec2 AB = B - A;
	Vec2 normal = AB.GetRotated90Degrees().GetNormalized();

	if (DotProduct2D(i, normal) > 0) {
		normal = normal * -1;
	}

	result.m_impactNormal = normal;

	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 forwardDir, float maxDist, AABB2 bounds)
{
	RaycastResult2D result;
	result.m_didImpact = false;
	
	if (bounds.IsPointInside(startPos))
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.0f;
		result.m_impactPos = startPos;
		result.m_impactNormal = -forwardDir;
		return result;
	}

	Vec2 invDir = Vec2(1.0f / forwardDir.x, 1.0f / forwardDir.y);

	// Calculate the intersection times for each axis
	float tmin = (bounds.m_mins.x - startPos.x) * invDir.x;
	float tmax = (bounds.m_maxs.x - startPos.x) * invDir.x;
	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (bounds.m_mins.y - startPos.y) * invDir.y;
	float tymax = (bounds.m_maxs.y - startPos.y) * invDir.y;
	if (tymin > tymax) std::swap(tymin, tymax);

	// Check for overlap
	if (tmin > tymax || tymin > tmax) 
	{
		return result;
	}

	// Calculate intersection times and impact distance
	float t_near = std::max(tmin, tymin);
	float t_far = std::min(tmax, tymax);

	if (t_near < 0.0f || t_far > maxDist) 
	{
		return result;
	}

	result.m_didImpact = true;
	result.m_impactDist = t_near;

	// Calculate impact position and normal
	result.m_impactPos = startPos + forwardDir * t_near;
	Vec2 impactOffset = result.m_impactPos - bounds.GetCenter();
	Vec2 halfDimensions = bounds.GetDimensions() * 0.5f;


	if (std::fabsf(impactOffset.x) > std::fabsf(impactOffset.y)) 
	{
		result.m_impactNormal = Vec2(std::copysign(1.0f, impactOffset.x), 0.0f);
	}
	else 
	{
		result.m_impactNormal = Vec2(0.0f, std::copysign(1.0f, impactOffset.y));
	}

	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
PlaneRaycastResult2D RaycastVsPlane2D(Vec2 const& rayStart, Vec2 const& rayForward, float maxDistance, Plane2D const& plane)
{
	PlaneRaycastResult2D planeRaycast;
	Vec2 endPos = rayStart + (rayForward * maxDistance);
	float planeDistance = plane.m_distance;
	float startPointDot = DotProduct2D(plane.m_planeNormal, rayStart);
	float endPointDot = DotProduct2D(plane.m_planeNormal, endPos);

	//-------------EARLY OUTS FOR IF RAY START AND END IS ON SAME SIDE-------------
	if (startPointDot > planeDistance && endPointDot > planeDistance)
	{
		return planeRaycast;
	}
	if (startPointDot < planeDistance && endPointDot < planeDistance)
	{
		return planeRaycast;
	}

	//---------SIMILAR TRIANGLES IN PLANE SPACE-------------
	float rayProject = DotProduct2D(rayForward, plane.m_planeNormal);
	float startPointAltitude = startPointDot - plane.m_distance;
	float impactDistance = fabsf(startPointAltitude / rayProject);
	if (impactDistance > maxDistance)
	{
		return planeRaycast;
	}
	planeRaycast.m_didImpact = true;
	planeRaycast.m_impactDist = impactDistance;
	planeRaycast.m_impactPos = rayStart + (rayForward * impactDistance);
	planeRaycast.m_impactNormal = rayForward.GetReflected(plane.m_planeNormal);

	if (startPointDot > planeDistance && endPointDot < planeDistance)
	{
		// If Ray goes from front of the plane to the back
		planeRaycast.m_isRayIntersectingFromFrontOfPlane = true;
	}
	else
	{
		// If Ray goes from back of the plane to the front
		planeRaycast.m_isRayIntersectingFromFrontOfPlane = false;
	}

	return planeRaycast;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2D hull)
{
	PlaneRaycastResult2D planesRaycast;
	RaycastResult2D convexShapeRaycast;
	float maximumEntryPointToShape = maxDist;
	if (hull.IsPointInside(startPos))
	{
		convexShapeRaycast.m_didImpact = true;
		convexShapeRaycast.m_impactPos = startPos;
		convexShapeRaycast.m_impactDist = 0.0f;
		convexShapeRaycast.m_impactNormal = -fwdNormal;
		return convexShapeRaycast;
	}
	for (int i = 0; i < hull.m_boundingPlanes.size(); i++)
	{
		//planesRaycast = RaycasVsPlane2D(startPos, fwdNormal, maxDist, hull.m_boundingPlanes[i]);
		planesRaycast = RaycastVsPlane2D(startPos, fwdNormal, maxDist, hull.m_boundingPlanes[i]);
		//------Check if Ray intersects each plane and it is the maximum entry point---------
		if (planesRaycast.m_didImpact && planesRaycast.m_isRayIntersectingFromFrontOfPlane && planesRaycast.m_impactDist < maximumEntryPointToShape)
		{
			bool pointIsOnConvexShape = true;
			//-------Check if the impacted plane point is behind all the other planes in the hull-------------
			//------i.e., Impact Position dot N < plane Distance
			//------So set it true and change to false if  Impact Position dot N > plane Distance
			for (int j = 0; j < hull.m_boundingPlanes.size(); j++)
			{
				Plane2D& hullPlane = hull.m_boundingPlanes[j];
				if (i == j)
				{
					continue;
				}
				float dotWithOtherPlane = DotProduct2D(planesRaycast.m_impactPos, hullPlane.m_planeNormal);
				if (dotWithOtherPlane > hullPlane.m_distance)
				{
					pointIsOnConvexShape = false;
				}
			}
			if (pointIsOnConvexShape)
			{
				convexShapeRaycast.m_didImpact = true;
				convexShapeRaycast.m_impactDist = planesRaycast.m_impactDist;
				convexShapeRaycast.m_impactPos = planesRaycast.m_impactPos;
				convexShapeRaycast.m_impactNormal = planesRaycast.m_impactNormal;
			}
		}
	}
	return convexShapeRaycast;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
{
	float ab = Interpolate(A, B, t);
	float bc = Interpolate(B, C, t);
	float cd = Interpolate(C, D, t);
	float abc = Interpolate(ab, bc, t);
	float bcd = Interpolate(bc, cd, t);
	float abcd = Interpolate(abc, bcd, t);
	return abcd;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float ab = Interpolate(A, B, t);
	float bc = Interpolate(B, C, t);
	float cd = Interpolate(C, D, t);
	float de = Interpolate(D, E, t);
	float ef = Interpolate(E, F, t);

	float abc = Interpolate(ab, bc, t);
	float bcd = Interpolate(bc, cd, t);
	float cde = Interpolate(cd, de, t);
	float def = Interpolate(de, ef, t);

	float abcd = Interpolate(abc, bcd, t);
	float bcde = Interpolate(bcd, cde, t);
	float cdef = Interpolate(cde, def, t);

	float abcde = Interpolate(abcd, bcde, t);
	float bcdef = Interpolate(bcde, cdef, t);

	float abcdef = Interpolate(abcde, bcdef, t);

	return abcdef;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStart2(float t)
{
	return t * t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStart3(float t)
{
	return t * t * t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStart4(float t)
{
	return t * t * t * t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStart5(float t)
{
	return t * t * t * t * t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStart6(float t)
{
	return t * t * t * t * t * t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStop2(float t)
{
	float inv_t = 1.0f - t;
	return 1.0f - (inv_t * inv_t);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStop3(float t)
{
	float inv_t = 1.0f - t;
	return 1.0f - inv_t * inv_t * inv_t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStop4(float t)
{
	float inv_t = 1.0f - t;
	return 1.0f - inv_t * inv_t * inv_t * inv_t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStop5(float t)
{
	float inv_t = 1.0f - t;
	return 1.0f - inv_t * inv_t * inv_t * inv_t * inv_t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStop6(float t)
{
	float inv_t = 1.0f - t;
	return 1.0f - inv_t * inv_t * inv_t * inv_t * inv_t * inv_t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStep3(float t)
{
	return ComputeCubicBezier1D(0, 0, 1, 1, t);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SmoothStep5(float t)
{
	return ComputeQuinticBezier1D(0, 0, 0, 1, 1, 1, t);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Hesitate3(float t)
{
	return ComputeCubicBezier1D(0, 1, 0, 1, t);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Hesitate5(float t)
{
	return ComputeQuinticBezier1D(0, 1, 0, 1, 0, 1, t);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float CustomFunkyFunction(float t)
{
	return t;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	posToTransform = (posToTransform.x * iBasis) + (posToTransform.y * jBasis) + translation;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformPositionXY3D(Vec3& positionToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	Vec2 pos2d(positionToTransform.x, positionToTransform.y);
	TransformPosition2D(pos2d, iBasis, jBasis, translationXY);
	positionToTransform.x = pos2d.x;
	positionToTransform.y = pos2d.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsBox(Vec2 const& rayStart, Vec2 const& rayForward, float maxDistance, AABB2 const& box)
{
	RaycastResult2D raycastResult;

	raycastResult.m_rayStartPosition = rayStart;
	raycastResult.m_impactDist = maxDistance;

	if (box.IsPointInside(rayStart)) 
{
		raycastResult.m_didImpact = true;
		raycastResult.m_forwardNormal = rayForward;
		raycastResult.m_impactPos = rayStart;
		raycastResult.m_impactDist = 0.0f;

		return raycastResult;
	}

	float const& left = box.m_mins.x;
	float const& bottom = box.m_mins.y;
	float const& right = box.m_maxs.x;
	float const& top = box.m_maxs.y;


	float rayLeftXEntry = (left - rayStart.x) / rayForward.x;
	float rayBottomYEntry = (bottom - rayStart.y) / rayForward.y;

	float rayRightXEntry = (right - rayStart.x) / rayForward.x;
	float rayTopYEntry = (top - rayStart.y) / rayForward.y;

	float rayXMin = (rayLeftXEntry < rayRightXEntry) ? rayLeftXEntry : rayRightXEntry;
	float rayXMax = (rayLeftXEntry > rayRightXEntry) ? rayLeftXEntry : rayRightXEntry;

	float rayYMin = (rayBottomYEntry < rayTopYEntry) ? rayBottomYEntry : rayTopYEntry;
	float rayYMax = (rayBottomYEntry > rayTopYEntry) ? rayBottomYEntry : rayTopYEntry;

	FloatRange xFloatRange(rayXMin, rayXMax);
	FloatRange yFloatRange(rayYMin, rayYMax);
	FloatRange allowedRange(0.0f, maxDistance);

	bool isOverlapping = xFloatRange.IsOverlappingWith(yFloatRange);
	if (isOverlapping) 
	{
		FloatRange overlappingRange = xFloatRange.GetOverlappingRange(yFloatRange);
		if (overlappingRange.IsOverlappingWith(allowedRange)) 
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = overlappingRange.m_min;
			raycastResult.m_impactPos = rayForward * overlappingRange.m_min + rayStart;

			if (xFloatRange.m_min > yFloatRange.m_min) 
			{
				if (rayForward.x < 0) {
					raycastResult.m_impactNormal = Vec2(1.0f, 0.0f);
				}
				else {
					raycastResult.m_impactNormal = Vec2(-1.0f, 0.0f);
				}
			}
			else {
				if (rayForward.y < 0) {
					raycastResult.m_impactNormal = Vec2(0.0f, 1.0f);
				}
				else {
					raycastResult.m_impactNormal = Vec2(0.0f, -1.0f);
				}
			}
		}
	}
	else {
		raycastResult.m_maxDistanceReached = true;
	}

	return raycastResult;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& center, float radius)
{
	float distanceSquared = GetDistanceSquared3D(point, center);
	return distanceSquared < (radius* radius);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------