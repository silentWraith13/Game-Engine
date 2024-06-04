#include "IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>
#include "Engine/Core/StringUtils.hpp"

IntVec2::IntVec2(const IntVec2& copyFrom)
    : x(copyFrom.x)
    , y(copyFrom.y)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2::IntVec2(int initialX, int initialY)
    : x(initialX)
    , y(initialY)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2::IntVec2(Vec2 XY)
	: x(static_cast<int>(XY.x))
	, y(static_cast<int>(XY.y))
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------------------------------------------------------------
float IntVec2::GetLength() const
{
    float length;
    length = sqrtf((static_cast<float>(x) * static_cast<float>(x)) + (static_cast<float>(y) * static_cast<float>(y)));
    
    return length;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int IntVec2::GetTaxicabLength() const
{
    return (abs(x) + abs(y));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int IntVec2::GetLengthSquared() const
{
    int squaredLength = (x * x) + (y * y);
    return  squaredLength;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float IntVec2::GetOrientationRadians() const
{
    float thetaRadians = atan2f(static_cast<float>(y), static_cast<float>(x));
  
    return thetaRadians;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float IntVec2::GetOrientationDegrees() const
{
    float thetaDegrees = Atan2Degrees(static_cast<float>(y), static_cast<float>(x));
    
    return thetaDegrees;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::GetRotated90Degrees() const
{  
    return IntVec2(-y, x);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
    
    return IntVec2(y, -x);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::Rotate90Degrees()
{
    int temp = x;
    x = -y;
    y = temp;
    
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::RotateMinus90Degrees()
{
    int temp = -x;
    x = y;
    y = temp;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::SetFromText(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, ',');
	x = static_cast<int>(atoi((string[0].c_str())));
	y = static_cast<int>(atoi((string[1].c_str())));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::SetFromTextWithDelimiter(const char* text, char Delimiter)
{
	Strings stringVec3 = SplitStringOnDelimiter(text, Delimiter);
	x = std::stoi(stringVec3[0]);
	y = std::stoi(stringVec3[1]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntVec2::operator==(const IntVec2& compare) const
{
    return (x == compare.x && y == compare.y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntVec2::operator!=(const IntVec2& compare) const
{
    return (x != compare.x || y != compare.y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntVec2::operator<(const IntVec2& other) const
{
	if (x < other.x)
		return true;
	else if (x == other.x)
		return y < other.y;
	else
		return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator+(const IntVec2& vecToAdd) const
{
    return IntVec2(x + vecToAdd.x, y + vecToAdd.y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-(const IntVec2& vecToSubtract) const
{
    return IntVec2(x - vecToSubtract.x, y - vecToSubtract.y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-() const
{
    return IntVec2(-x, -y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*(float uniformScale) const
{
    return IntVec2(x * static_cast<int> (uniformScale), y * static_cast<int> (uniformScale));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*(const IntVec2& vecToMultiply) const
{
    return IntVec2(vecToMultiply.x,vecToMultiply.y);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator/(float inverseScale) const
{
    return IntVec2(x / static_cast<int>(inverseScale), y/static_cast<int>(inverseScale));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::operator+=(const IntVec2& vecToAdd)
{
    x += vecToAdd.x;
    y += vecToAdd.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

void IntVec2::operator-=(const IntVec2& vecToSubtract)
{
    x -= vecToSubtract.x;
    y -= vecToSubtract.y;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::operator*=(const float uniformScale)
{
    x *= static_cast<int> (uniformScale);
    y *= static_cast<int> (uniformScale);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::operator/=(const float uniformDivisor)
{
    y /= static_cast<int> (uniformDivisor);
    x /= static_cast<int> (uniformDivisor);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec2::operator=(const IntVec2& copyFrom)
{
    x = copyFrom.x;
    y = copyFrom.y;
}
