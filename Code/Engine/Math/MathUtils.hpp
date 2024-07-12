#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vec2;
struct Vec4;
struct IntVec2;
struct AABB2;
struct OBB2D;
struct Capsule2;
struct LineSegment2;
class  ConvexHull2D;
class  ConvexPoly2D;
class  RandomNumberGenerator;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct RaycastResult2D
{
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec2	m_impactPos;
	Vec2	m_impactNormal;
	Vec2	m_rayStartPosition;
	Vec2	m_rayDirection;
	float	m_rayLength = 1.0f;
	bool	m_maxDistanceReached = false;
	Vec2	m_forwardNormal = Vec2::ZERO;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct PlaneRaycastResult2D : public RaycastResult2D
{
	bool	m_isRayIntersectingFromFrontOfPlane = false;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct RaycastResult3D
{
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;
	Vec3	m_rayStartPosition;
	Vec3	m_rayDirection;
	float	m_rayLength = 1.0f;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class BillboardType
{
	NONE = -1,
	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	CUSTOM_PROJECTILE,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//Angle Utilities
float					ConvertDegreesToRadians(float degrees);
float					ConvertRadiansToDegrees(float radians);
float					CosDegrees(float degrees);
float					SinDegrees(float degrees);
float					Atan2Degrees(float y, float x);
float					GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float					GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
float					GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);

//Clamp and Lerp
float					GetClamped(float value,float minValue,float maxValue);
int						GetClamped(int value, int minValue, int maxValue);
float					GetClampedZeroToOne(float value);
float					Interpolate(float start,float end,float fractionTowardsEnd);
float					GetFractionWithinRange(float value, float rangeStart,float rangeEnd);
float					RangeMap(float inValue,float inStart,float inEnd, float outStart,float outEnd);
int						Int_Interpolate(int start, int end, int fractionTowardsEnd);
int						RoundDownToInt(float value);
Vec3					ScaleVec3(Vec3 const& value, float posX, float posY, float posZ, float negX, float negY, float negZ);
Vec3					GetClampedVec3(Vec3 const& value, Vec3 const& minValue, Vec3 const& maxValu);
EulerAngles				InterpolateEulerAngles(EulerAngles const& start, EulerAngles const& end, float factor);
Vec2					InterpolateVec2(const Vec2& start, const Vec2& end, float fractionTowardsEnd);
float					RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
Rgba8					Interpolate(Rgba8 start, Rgba8 end, float factor); 
Rgba8					HueToRgba8(float hue);
Rgba8					GetRandomColor(RandomNumberGenerator rng);

//Dot and cross
float					DotProduct2D(Vec2 const& a, Vec2 const& b);
float					DotProduct3D(Vec3 const& a, Vec3 const& b);
float					DotProduct4D(Vec4 const& a, Vec4 const& b);
float					CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3					CrossProduct3D(Vec3 const& a, Vec3 const& b);

//Distance and projection utilities
float					GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float					GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
float					GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float					GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float					GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float					GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float					GetDistanceFromPointToLineSegment2D(Vec2 const& point, LineSegment2 const& lineSegment);
float					GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
int						GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB); 
Vec2 const				GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
Vec3 const				GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);
Vec3 const				GetPerpendicularComponent3D(Vec3 const& vectorToAdjust, Vec3 const& axis);
Vec3					GetProjectedOntoPlane(Vec3 const& vectorToProject, Vec3 const& planeNormal);

//Geometric query utilities
bool					DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool					DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool					DoAAB3DsOverlap(AABB3 const& first, AABB3 const& second);

bool					IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool					IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool					IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool					IsPointInsideDirectedCone3D(const Vec3& point, const Vec3& coneTip, const Vec3& coneForwardNormal, float coneApertureDegrees, float coneRadius);
bool					IsPointInsideOBB2D(Vec2 const& point, OBB2D const& orientedBox);
bool					IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);			
bool					IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule, float radius);
bool					IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool					IsPointInsideLineSegment2D(Vec2 const& point, LineSegment2 const& lineSegment);
bool					IsPointInsideCapsule3D(Vec3 const& blockWorldCenter, Vec3 const& capsuleWorldStart, Vec3 const&  capsuleWorldEnd, float capsuleRadius);
bool					IsPointInsideHexagon2D(const Vec2& P, const Vec2& center, float radius);
bool					IsPointInsideConvexPoly2D(Vec2 const& refPoint, ConvexPoly2D const& convexPoly, float tolerance = 0.025f);
bool					IsPointInsideConvexHull2D(Vec2 const& refPoint, ConvexHull2D const& convexHull, float tolerance = 0.025f);
bool					IsPointInsideSphere3D(Vec3 const& point, Vec3 const& center, float radius);

Vec2 const				GetNearestPointInCapsule2D(Vec2 const& referencePosition, Vec2& boneStart, Vec2& boneEnd, float radius);
Vec2 const				GetNearestPointInCapsule2D(Vec2 const& referencePosition, Capsule2& capsule);
Vec2 const				GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius);
Vec2 const				GetNearestpointOnOBB2D(Vec2 const& referencePosition, OBB2D const& orientedBox);
Vec2 const				GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd);
Vec2 const				GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& lineSegment);
Vec2 const				GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
Vec2 const				GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, LineSegment2 const& infiniteLine);
Vec2 const				GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2& box);

bool					PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool					PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool					PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool					PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);

void					BounceDiscOffFixedDisc2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, Vec2 const& fixedCenter, float fixedRadius, float elasticity);
void					BounceDiscOffDisc2D(Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticity);
void					BounceDiscOffOfPoint2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, Vec2 const& fixedCenter, float elasticity);
void					BounceOffFixedOBB2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, OBB2D const& box, float elasticity);
void					BounceOffFixedAABB2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, AABB2& box, float elasticity);
void					BounceOffFixedCapsule2D(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, Capsule2& capsule, float elasticity);

//Transformation utilities
void					TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);
void					TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void					TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void					TransformPositionXY3D(Vec3& positionToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);

//Normalized and de-normalized byte
float					NormalizeByte(unsigned char byteValue);
unsigned char			DenormalizeByte(float zeroToOne);

//Billboard Matrix
Mat44					GetBillboardMatrix(BillboardType billboardType, Mat44 const& camMatrix, const Vec3& billboardPos, const Vec2& billboardScale = Vec2(1.0f, 1.0f));

//Raycast functions for different geometries
RaycastResult2D			RaycastVsDisc2D(Vec2 startPos, Vec2 forwardDir, float maxDist, Vec2 discCenter, float discRadius);
RaycastResult2D			RaycastVsLineSegment2D(Vec2 startPos, Vec2 forwardDir, float maxDist, Vec2 lineStart, Vec2 lineEnd);
RaycastResult2D			RaycastVsAABB2D(Vec2 startPos, Vec2 forwardDir, float maxDist, AABB2 bounds);
RaycastResult2D			RaycastVsConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2D hull);
PlaneRaycastResult2D	RaycastVsPlane2D(Vec2 const& rayStart, Vec2 const& rayForward, float maxDistance, Plane2D const& plane);
RaycastResult2D			RaycastVsBox(Vec2 const& rayStart, Vec2 const& rayForward, float maxDistance, AABB2 const& box);

//Cubic Bezier and smoothing functions
float					ComputeCubicBezier1D(float A, float B, float C, float D, float t);
float					ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);
float					SmoothStart2(float t);
float					SmoothStart3(float t);
float					SmoothStart4(float t);
float					SmoothStart5(float t);
float					SmoothStart6(float t);
float					SmoothStop2(float t);
float					SmoothStop3(float t);
float					SmoothStop4(float t);
float					SmoothStop5(float t);
float					SmoothStop6(float t);
float					SmoothStep3(float t);
float					SmoothStep5(float t);
float					Hesitate3(float t);
float					Hesitate5(float t);
float					CustomFunkyFunction(float t);

float					Sigmoid(float x);
//--------------------------------------------------------------------------------------------------------------------------------------------------------




