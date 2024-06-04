#include "Engine/Math/Splines.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
CubicBezierCurve2D::CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos)
	:m_a(startPos), m_b(guidePos1), m_c(guidePos2), m_d(endPos)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
CubicBezierCurve2D::CubicBezierCurve2D()
	: m_a(Vec2(0.f, 0.f)), m_b(Vec2(0.f, 0.f)), m_c(Vec2(0.f, 0.f)), m_d(Vec2(0.f, 0.f))
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::EvaluateAtParametric(float parametricZeroToOne)
{
	Vec2 ab = InterpolateVec2(m_a, m_b, parametricZeroToOne);
	Vec2 bc = InterpolateVec2(m_b, m_c, parametricZeroToOne);
	Vec2 cd = InterpolateVec2(m_c, m_d, parametricZeroToOne);
	Vec2 abc = InterpolateVec2(ab, bc, parametricZeroToOne);
	Vec2 bcd = InterpolateVec2(bc, cd, parametricZeroToOne);
	Vec2 abcd = InterpolateVec2(abc, bcd, parametricZeroToOne);
	return abcd;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float CubicBezierCurve2D::GetApproximateLength(int numSubDivisions /*= 64*/)
{
	float approxLength = 0.0f;
	Vec2 prevPoint = m_a;

	for (int i = 1; i <= numSubDivisions; ++i)
	{
		float t = (float)i / (float)numSubDivisions;
		Vec2 currentPoint = EvaluateAtParametric(t);
		float segmentLength = GetDistance2D(prevPoint, currentPoint);
		approxLength += segmentLength;
		prevPoint = currentPoint;
	}

	return approxLength;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubDivisions /*= 64*/)
{
	for (int i = 0; i < numSubDivisions; ++i)
	{
		float t1 = (float)(i) / (float)numSubDivisions;
		float t2 = (float)(i + 1) / (float)numSubDivisions;

		Vec2 point1 = EvaluateAtParametric(t1);
		Vec2 point2 = EvaluateAtParametric(t2);

		float segmentLength = GetDistance2D(point1, point2);

		if (distanceAlongCurve <= segmentLength)
		{
			float segmentT = distanceAlongCurve / segmentLength;
			Vec2 finalPos = point1 + (point2 - point1) * segmentT;
			return finalPos;
		}
		else
		{
			distanceAlongCurve -= segmentLength;
		}
	}

	return EvaluateAtParametric(1.0f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void CubicBezierCurve2D::SetStartVelocity(Vec2 startVel)
{
	m_b = m_a + startVel * (1.f / 3.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::GetStartVelocity()
{
	return 3.f * (m_b - m_a);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void CubicBezierCurve2D::SetEndVelocity(Vec2 endVel)
{
	m_c = m_d - endVel * (1.f / 3.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::GetEndVelocity()
{
	return 3.f * (m_d - m_c);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::EvaluateTangentAtParametric(float t)
{
	// The derivative of a cubic Bezier curve is a linear combination of two vectors
	Vec2 P0 = m_a * (1.f - t) * (1.f - t) + m_b * 2.f * t * (1.f - t) + m_c * t * t;
	Vec2 P1 = m_b * (1.f - t) * (1.f - t) + m_c * 2.f * t * (1.f - t) + m_d * t * t;

	// The tangent is the derivative normalized
	Vec2 tangent = P1 - P0;
	tangent.Normalize();
	return tangent;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// //--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
CatmullRomSpline2D::CatmullRomSpline2D()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
CatmullRomSpline2D::CatmullRomSpline2D(std::vector<Vec2> positions)
{
	if (positions.size() == 0)
	{
		return;
	}

	if (positions.size() == 1)
	{
		ERROR_AND_DIE("Cannot create a CatmullRomSpline with exactly 1 point, must be at least 2 or empty");
	}

	int numCurveSections = static_cast<int>(positions.size()) - 1;
	m_curvesSections.resize(numCurveSections);

	for (int i = 0; i < numCurveSections; ++i)
	{
		CubicBezierCurve2D& curve = m_curvesSections[i];

		curve.m_a = positions[i];
		curve.m_d = positions[i + 1];

		// Calculate start and end velocities
		Vec2 startVel = Vec2(0.f, 0.f);
		Vec2 endVel = Vec2(0.f, 0.f);
		if (i > 0)
		{
			// This isn't the first curve, so we can look back at the previous curve to calculate the start velocity.
			startVel = ((positions[i] - positions[i - 1]) + (positions[i + 1] - positions[i])) * 0.5f;
		}

		if (i < numCurveSections - 1)
		{
			// This isn't the last curve, so we can look forward at the next curve to calculate the end velocity.
			endVel = ((positions[i + 1] - positions[i]) + (positions[i + 2] - positions[i + 1])) * 0.5f;
		}

		curve.SetStartVelocity(startVel);
		curve.SetEndVelocity(endVel);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CatmullRomSpline2D::EvaluateAtParametric(float splineT)
{
	// Calculate the total number of curve sections
	int numCurveSections = static_cast<int>(m_curvesSections.size());

	// Determine the curve index and the local T value for that curve
	int curveIndex = RoundDownToInt(splineT);
	float localT = splineT - curveIndex;
	if (curveIndex < 0)
	{
		curveIndex = 0;
		localT = 0.f;
	}

	if (curveIndex >= numCurveSections)
	{
		curveIndex = numCurveSections - 1;
		localT = 1.f;
	}

	// Evaluate the position on the corresponding Bezier curve
	return m_curvesSections[curveIndex].EvaluateAtParametric(localT);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float CatmullRomSpline2D::GetApproximateLength(int numSubDivisions /*= 64*/)
{
	float approxLength = 0.0f;

	for (int i = 0; i < m_curvesSections.size(); ++i)
	{
		approxLength += m_curvesSections[i].GetApproximateLength(numSubDivisions);
	}

	return approxLength;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CatmullRomSpline2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubDivisions /*= 64*/)
{
	float currentLength = 0.f;
	for (int i = 0; i < m_curvesSections.size(); ++i)
	{
		float curveLength = m_curvesSections[i].GetApproximateLength(numSubDivisions);

		if (currentLength + curveLength >= distanceAlongCurve)
		{
			float remainingLength = distanceAlongCurve - currentLength;
			return m_curvesSections[i].EvaluateAtApproximateDistance(remainingLength, numSubDivisions);
		}
		else
		{
			currentLength += curveLength;
		}
	}

	// If the input distance is greater than the total length, return the endpoint
	return m_curvesSections[m_curvesSections.size() - 1].EvaluateAtParametric(1.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 CatmullRomSpline2D::EvaluateTangentAtParametric(float t)
{
	int curveIndex = static_cast<int>(t);
	if (curveIndex >= m_curvesSections.size())
	{
		curveIndex = static_cast<int>(m_curvesSections.size()) - 1;
	}

	float localT = t - curveIndex;
	return m_curvesSections[curveIndex].EvaluateTangentAtParametric(localT);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------