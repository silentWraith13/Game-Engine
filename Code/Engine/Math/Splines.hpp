#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class CubicBezierCurve2D
{
public:
	CubicBezierCurve2D();
	CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos);
	Vec2 EvaluateAtParametric(float parametricZeroToOne);
	float GetApproximateLength(int numSubDivisions = 64);
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubDivisions = 64);
	void SetStartVelocity(Vec2 startVel);
	Vec2 GetStartVelocity();
	void SetEndVelocity(Vec2 endVel);
	Vec2 GetEndVelocity();
	Vec2 EvaluateTangentAtParametric(float t);

	Vec2 m_a; //Start pos.
	Vec2 m_b; //first guide point
	Vec2 m_c; //Second guide point
	Vec2 m_d;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class CatmullRomSpline2D
{
public:
	CatmullRomSpline2D();
	CatmullRomSpline2D(std::vector<Vec2> positions);
	Vec2 EvaluateAtParametric(float parametricZeroToOne);
	float GetApproximateLength(int numSubDivisions = 64);
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubDivisions = 64);
	Vec2 EvaluateTangentAtParametric(float t);

	std::vector<CubicBezierCurve2D> m_curvesSections;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------