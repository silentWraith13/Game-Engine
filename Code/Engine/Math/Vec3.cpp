#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <cmath>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3::Vec3(float initialX, float initialY, float initialZ)

    :x(initialX)
    ,y(initialY)
    , z(initialZ)
{    
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3::Vec3(Vec2 const& initialXY)
    :x(initialXY.x)
    ,y(initialXY.y)
    ,z(0.0f)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3::Vec3(IntVec3 const& copyFrom)
	: x(static_cast<float>(copyFrom.x))
	, y(static_cast<float>(copyFrom.y))
    , z(static_cast<float>(copyFrom.z))
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3::Vec3(Vec2 const& xy, float z)
    :x(xy.x), y(xy.y), z(z)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length /*= 1.0f*/)
{
	float x = length * cosf(latitudeRadians) * cosf(longitudeRadians);
	float y = length * cosf(latitudeRadians) * sinf(longitudeRadians);
	float z = length * (-sinf(latitudeRadians));
	return Vec3(x, y, z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length /*= 1.0f*/)
{
    float x = length * CosDegrees(latitudeDegrees) * CosDegrees(longitudeDegrees);
    float y = length * CosDegrees(latitudeDegrees) * SinDegrees(longitudeDegrees);
    float z = length * (-SinDegrees(latitudeDegrees));
	return Vec3(x, y, z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec3::GetLength() const
{
    float length = sqrtf((x * x) + (y * y) + (z * z));
    return length;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec3::GetLengthXY() const
{
    float lengthXY = sqrtf((x * x) + (y * y));
    
    return lengthXY;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec3::GetLengthSquared() const
{
    float length = GetLength();
    float lengthSquared = length * length;
    
    return lengthSquared;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec3::GetLengthXYSquared() const
{
    float lengthXY = GetLengthXY();
    float lengthSquaredXY = lengthXY * lengthXY;
   
    return lengthSquaredXY;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZRadians() const
{
    float thetaRadians = atan2f(y, x);
    
    return thetaRadians;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZDegrees() const
{
    float thetaDegrees = Atan2Degrees(y, x);
    
    return thetaDegrees;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
    float thetaRadians = GetAngleAboutZRadians();
    float length = GetLengthXY();
    
    thetaRadians += deltaRadians;
    
    float x1 = length * cosf(thetaRadians);
    float y1 = length * sinf(thetaRadians);
    
    return Vec3(x1, y1, z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
    float thetaDegrees = Atan2Degrees(y, x);
    float length = GetLengthXY();
    
    thetaDegrees += deltaDegrees;
    
    float x1 = length * CosDegrees(thetaDegrees);
    float y1 = length * SinDegrees(thetaDegrees);
    
    return Vec3(x1, y1, z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetClamped(float maxLength) const     
{
    float length = GetLength();

    if (length > maxLength)
    {
        float scale = maxLength / length;
        return Vec3(x * scale, y * scale,z*scale);
    }
    
    return *this;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetNormalized() const        
{
	float length = GetLength();

	if (length > 1e-6) 
    { 
		float scale = 1.f / length;
		return Vec3(x * scale, y * scale, z * scale);
	}

	return Vec3(1.0f, 0.0f, 0.0f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::Normalize()
{
	float length = GetLength();

	if (length > 0.0f)
	{
		float scale = 1.0f / length;
		x *= scale;
		y *= scale;
        z *= scale;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Vec3::GetMin(const Vec3& a, const Vec3& b)
{
    return Vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Vec3::GetMax(const Vec3& a, const Vec3& b)
{
    return Vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Vec3::operator==(Vec3 const& compare) const
{
    return (x == compare.x && y == compare.y && z == compare.z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Vec3::operator!=(Vec3 const& compare) const
{
    return (x != compare.x || y != compare.y || z !=compare.z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator+(Vec3 const& vecToAdd) const
{
    return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator-(Vec3 const& vecToSubtract) const
{
    return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator*(float uniformScale) const
{
    return Vec3(x * uniformScale, y * uniformScale, z*uniformScale);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Vec3::operator*(Vec3 const& nonUniformScales)
{
    return Vec3(x * nonUniformScales.x, y * nonUniformScales.y, z * nonUniformScales.z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::SetFromText(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, ',');
	x = static_cast<float>(atof((string[0].c_str())));
	y = static_cast<float>(atof((string[1].c_str())));
    z = static_cast<float>(atof((string[2].c_str())));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Vec3::SetFromString(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, ',');
	x = static_cast<float>(atof((string[0].c_str())));
	y = static_cast<float>(atof((string[1].c_str())));
	z = static_cast<float>(atof((string[2].c_str())));
    return Vec3(x, y, z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator/(float inverseScale) const
{
    return Vec3(x / inverseScale, y / inverseScale, z/inverseScale);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::operator+=(Vec3 const& vecToAdd)
{
    x += vecToAdd.x;
    y += vecToAdd.y;
    z += vecToAdd.z;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::operator-=(Vec3 const& vecToSubtract)
{
    x -= vecToSubtract.x;
    y -= vecToSubtract.y;
    z -= vecToSubtract.z;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::operator*=(float uniformScale)
{
    x *= uniformScale;
    y *= uniformScale;
    z *= uniformScale;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::operator/=(float uniformDivisor)
{
    y /= uniformDivisor;
    x /= uniformDivisor;
    z /= uniformDivisor;
}
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::operator=(Vec3 const& copyFrom)
{

    x = copyFrom.x;
    y = copyFrom.y;
    z = copyFrom.z;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const operator*(float uniformScale, Vec3 const& vecToScale)
{
    return Vec3(uniformScale * (vecToScale.x), uniformScale * (vecToScale.y), uniformScale * (vecToScale.z));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::SetFromTextWithSpace(const char* text)
{
	Strings stringVec2 = SplitStringOnDelimiter(text, ' ');
	x = std::stof(stringVec2[0]);
	y = std::stof(stringVec2[1]);
	z = std::stof(stringVec2[2]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Vec3::SetFromTextWithDelimiter(const char* text, char delimiter)
{
	Strings stringVec2 = SplitStringOnDelimiter(text, delimiter);
	x = std::stof(stringVec2[0]);
	y = std::stof(stringVec2[1]);
	z = std::stof(stringVec2[2]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------