#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>
#include "Engine/Core/StringUtils.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const Vec2 Vec2::ZERO = Vec2(0.0f, 0.0f);
const Vec2 Vec2::ONE = Vec2(1.0f, 1.0f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
 	: x( copy.x )
	, y( copy.y)
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x(initialX)
	, y(initialY)
{
}

Vec2::Vec2(IntVec2 XY)
	:x(static_cast<float>(XY.x))
	,y(static_cast<float>(XY.y))
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2::Vec2(const Vec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	float x = length * cos(orientationRadians);
	float y = length * sin(orientationRadians);
	
	return Vec2(x, y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float x = length * CosDegrees(orientationDegrees);
	float y = length * SinDegrees(orientationDegrees);
	
	return Vec2(x, y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Vec2::MakeFromMins(Vec2 const& mins, Vec2 const& other)
{
	float minX = fminf(mins.x, other.x);
	float minY = fminf(mins.y, other.y);
	return Vec2(minX, minY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Vec2::MakeFromMaxs(Vec2 const& mins, Vec2 const& other)
{
	float minX = fmaxf(mins.x, other.x);
	float minY = fmaxf(mins.y, other.y);
	return Vec2(minX, minY);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec2::GetLength() const
{
	float length = sqrtf((x * x) + (y * y));

	return length;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec2::GetLengthSquared() const
{
	float lengthSquared = (x * x) + (y * y);
	return lengthSquared;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec2::GetOrientationRadians() const
{
	float thetaRadians = atan2f(y, x);
	return thetaRadians;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec2::GetOrientationDegrees() const
{
	float thetaDegrees = Atan2Degrees(y, x);
	
	return thetaDegrees;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetRotated90Degrees() const
{
	float length = GetLength();
	float thetaDegrees = GetOrientationDegrees();
	
	thetaDegrees += 90.0f;
	
	float x1 = length * CosDegrees(thetaDegrees);
	float y1 = length * SinDegrees(thetaDegrees);
	
	return Vec2(x1, y1);

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	float length = GetLength();
	float thetaDegrees = GetOrientationDegrees();
	
	thetaDegrees -= 90.0f;
	
	float x1 = length * CosDegrees(thetaDegrees);
	float y1 = length * SinDegrees(thetaDegrees);
	
	return Vec2(x1, y1);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	float length = GetLength();
	float thetaRadians = GetOrientationRadians();
	
	thetaRadians += deltaRadians;
	
	float x1 = length * cos(thetaRadians);
	float y1 = length * sin(thetaRadians);
	
	return Vec2(x1, y1);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	float length = GetLength();
	float thetaDegrees = GetOrientationDegrees();
	
	thetaDegrees += deltaDegrees;
	
	float x1 = length * CosDegrees(thetaDegrees);
	float y1 = length * SinDegrees(thetaDegrees);
	
	return Vec2(x1, y1);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetClamped(float maxLength) const    
{
	float length = GetLength();

	if (length > maxLength)
	{
		float scale = maxLength / length;
		return Vec2(x * scale, y * scale);
	}

	return *this;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

Vec2 const Vec2::GetNormalized() const  
{
	float length = GetLength();

	if (length == 0.f)
	{
		return Vec2(0.f, 0.f);
	}

	float inverseLength = 1.0f / length;
	return Vec2(x * inverseLength, y * inverseLength);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetReflected(Vec2 const& bounceSurfaceNormal) const
{
	Vec2 V(x,y);
	Vec2 Vn = DotProduct2D(V,bounceSurfaceNormal) * bounceSurfaceNormal;
	Vec2 Vt = V - Vn;
	return (Vt - Vn);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float length = GetLength();
	float thetaRadians = GetOrientationRadians();
	
	thetaRadians = newOrientationRadians;
	
	x = length * cos(thetaRadians);
	y = length * sin(thetaRadians);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float length = GetLength();
	float thetaDegrees = GetOrientationDegrees();
	
	thetaDegrees = newOrientationDegrees;
	
	x = length * CosDegrees(thetaDegrees);
	y = length * SinDegrees(thetaDegrees);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	float length = GetLength();
	float thetaRadians = GetOrientationRadians();
	
	length = newLength;
	thetaRadians = newOrientationRadians;
	
	x = newLength * cos(thetaRadians);
	y = newLength * sin(thetaRadians);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	float length = GetLength();
	float thetaDegrees = GetOrientationDegrees();
	
	length = newLength;
	thetaDegrees = newOrientationDegrees;
	
	x = newLength * CosDegrees(thetaDegrees);
	y = newLength * SinDegrees(thetaDegrees);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::Rotate90Degrees()
{
	float thetaDegrees = GetOrientationDegrees();	
	float length = GetLength();
	
	thetaDegrees += 90.0f;
	
	x = length * CosDegrees(thetaDegrees);
	y = length * SinDegrees(thetaDegrees);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::RotateMinus90Degrees()
{
	float thetaDegrees = GetOrientationDegrees();	
	float length = GetLength();
	
	thetaDegrees -= 90.0f;
	
	x = length * CosDegrees(thetaDegrees);
	y = length * SinDegrees(thetaDegrees);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::RotateRadians(float deltaRadians)
{
	float thetaRadians = GetOrientationRadians();
	float length = GetLength();
	
	thetaRadians += deltaRadians;
	
	x = length * cos(thetaRadians);
	y = length * sin(thetaRadians);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::RotateDegrees(float deltaDegrees)
{
	float thetaDegrees = Atan2Degrees(y, x);
	float length = GetLength();
	
	thetaDegrees += deltaDegrees;
	
	x = length * CosDegrees(thetaDegrees);
	y = length * SinDegrees(thetaDegrees);
	
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::SetLength(float newLength)
{
	float length = GetLength();
	if (newLength == 0.f || length == 0.f)
	{
		x = 0.f;
		y = 0.f;
		return;
	}

	float scaleLength =  newLength / length;
	
	x *= scaleLength;
	y *= scaleLength;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::ClampLength(float maxLength)
{
	float length = GetLength();
	
	if (length > maxLength)
	{
		float scale = maxLength / length;
		x *= scale;
		y *= scale;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::Normalize()
{
	float length = GetLength();
	
	if (length > 0.0f)
	{
		float scale = 1.0f / length;
		x *= scale;
		y *= scale;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec2::NormalizeAndGetPreviousLength()
{
	float previousLength = GetLength();
	Normalize();
	
	return previousLength;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::Reflect(Vec2 const& bounceSurfaceNormal)
{
	*this = GetReflected(bounceSurfaceNormal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec2::SetFromText(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, ',');
	x = static_cast<float>(atof((string[0].c_str() ) ) );
	y = static_cast<float>(atof((string[1].c_str())));
}

//-----------------------------------------------------------------------------------------------
void Vec2::SetFromText(const char* text, char DelimiterToSplitOn)
{
	Strings stringVec2 = SplitStringOnDelimiter(text, DelimiterToSplitOn);
	x = std::stof(stringVec2[0]);
	y = std::stof(stringVec2[1]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator+( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2(-x,-y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2(x*uniformScale,y*uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x,y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( x/inverseScale, y/inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;

}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	y /= uniformDivisor;
	x /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(uniformScale * (vecToScale.x), uniformScale * (vecToScale.y));
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return (x==compare.x && y==compare.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return (x != compare.x || y != compare.y);
}


