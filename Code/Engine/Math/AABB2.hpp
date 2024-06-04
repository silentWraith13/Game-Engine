#pragma once
#include "Engine/Math/Vec2.hpp"
#include"Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Shape2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct AABB2 : public Shape2D
{
public:
	Vec2 m_mins;
	Vec2 m_maxs;

//Construction/deconstruction
	~AABB2(){}
	AABB2(){}
	AABB2(AABB2 const& copyFrom);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(Vec2 const& mins, Vec2 const& maxs);
	bool		operator==(const AABB2& compare) const;		// vec2 == vec2

//Accessors(const methods)
	bool       IsPointInside(Vec2 const& point) const;
	Vec2 const GetCenter() const;
	Vec2 const GetDimensions() const;
	Vec2 const GetNearestPoint(Vec2 const& referencePosition) const;
	Vec2 const GetPointAtUV(Vec2 const& uv) const;        //uv=(0,0) is at mins; uv=(1,1) is at maxs
	Vec2 const GetUVForPoint(Vec2 const& point) const;    //uv=(.5,.5) at center; u or v outside [0,1] extrapolated
	AABB2      GetBoxAtUV(const Vec2& uv1, const Vec2& uv2) const;
//Mutators(non-const methods)
	void      Translate(Vec2 const& translationToApply);
	void      SetCenter(Vec2 const& newCenter);
	void      SetDimensions(Vec2 const& newDimensions);
	void      StretchToIncludePoint(Vec2 const& point);  //does minmal stretching required(none if already on point)
	
	void AddPadding(float paddingX, float paddingY);
	AABB2 GetBoxWithin(Vec2 const& point1, Vec2 const& point2) const;

	static AABB2 ZERO_TO_ONE;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------