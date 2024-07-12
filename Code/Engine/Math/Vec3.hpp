#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/StringUtils.hpp"

//-----------------------------------------------------------------------------------------------
struct Vec3
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.0f;

public:

	Vec3() = default;
	explicit Vec3(float initialX, float initialY, float initialZ);
	explicit Vec3(Vec2 const& initialXY);
	Vec3(IntVec3 const& copyFrom);
	explicit Vec3(Vec2 const& xy, float z);

	//Make from polar
	static Vec3 const	MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length = 1.0f);
	static Vec3 const	MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length = 1.0f);

	//Accessors(const methods)
	float				GetLength() const;
	float				GetLengthXY() const;
	float				GetLengthSquared() const;
	float				GetLengthXYSquared() const;
	float				GetAngleAboutZRadians() const;
	float				GetAngleAboutZDegrees() const;
	Vec3 const			GetRotatedAboutZRadians(float deltaRadians) const;
	Vec3 const			GetRotatedAboutZDegrees(float deltaDegrees) const;
	Vec3 const			GetClamped(float maxLength) const;
	Vec3				GetMin(const Vec3& a, const Vec3& b);
	Vec3				GetMax(const Vec3& a, const Vec3& b);
	Vec3 const			GetNormalized() const;
	void				Normalize();

	void				SetFromText(char const* text);
	Vec3				SetFromString(char const* text);
	void				SetFromTextWithSpace(const char* text);
	void				SetFromTextWithDelimiter(const char* text, char delimiter);

	//Operators(const)
	bool				operator == (Vec3 const& compare)const;                                  //vec3 = vec3;
	bool				operator != (Vec3 const& compare)const;                                  //vec3 != vec3;
	Vec3 const			operator+(Vec3 const& vecToAdd) const;									//vec3 + vec3;
	Vec3 const			operator-(Vec3 const& vecToSubtract) const;                              //vec3 - vec3;
	Vec3 const			operator*(float uniformScale) const;                                     //vec3 * float;
	Vec3 const			operator/(float inverseScale) const;                                     //vec / float;

//Operators(Self-Mutating)
	void				operator+=(Vec3 const& vecToAdd);
	void				operator-=(Vec3 const& vecToSubtract);
	void				operator*=(float uniformScale);
	void				operator/=(float uniformDivisor);
	void				operator=(Vec3 const& copyFrom);
	Vec3				operator*(Vec3 const& nonUniformScales);

	//Standalone functions which are conceptually part, but not actually a part of Vec3
	friend Vec3 const	operator*(float uniformScale, Vec3 const& vecToScale);           //float * Vec3     
	
};



