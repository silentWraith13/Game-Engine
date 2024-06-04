#include "IntVec3.hpp"
#include "Vec3.hpp"
#include <math.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const IntVec3 IntVec3::ZERO = IntVec3(0, 0, 0);
const IntVec3 IntVec3::INVALID = IntVec3(UINT_MAX, UINT_MAX, UINT_MAX);
const IntVec3 IntVec3::ONE = IntVec3(1, 1, 1);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3::IntVec3(IntVec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3::IntVec3(int initialX, int initialY, int initialZ)
{
	x = initialX;
	y = initialY;
	z = initialZ;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3::IntVec3(Vec3 const& copyFrom)
{
	x = RoundDownToInt(copyFrom.x);
	y = RoundDownToInt(copyFrom.y);
	z = RoundDownToInt(copyFrom.z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float IntVec3::GetLength() const
{
	return (float)sqrtf((float)x * x + y * y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int IntVec3::GetLengthSquared() const
{
	return (x * x + y * y + z * z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int IntVec3::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3 const IntVec3::GetRotated90Degrees() const
{
	return IntVec3(-y, x, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3 const IntVec3::GetRotatedMinus90Degrees() const
{
	return IntVec3(y, -x, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float IntVec3::GetOrientationDegrees() const
{
	return Atan2Degrees((float)y, (float)x);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float IntVec3::GetOrientationRadians() const
{
	return atan2f((float)y, (float)x);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec3::Rotate90Degrees()
{
	IntVec3 rotatatedVec = IntVec3(-y, x, 0);
	x = rotatatedVec.x;
	y = rotatatedVec.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec3::RotateMinus90Degrees()
{
	IntVec3 rotatatedVec = IntVec3(y, -x, 0);
	x = rotatatedVec.x;
	y = rotatatedVec.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntVec3::operator==(const IntVec3& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z)
	{
		return true;
	}
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool IntVec3::operator!=(const IntVec3& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z)
	{
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3	IntVec3::operator+(const IntVec3& IntVec3ToAdd)
{
	return IntVec3(IntVec3ToAdd.x + x, IntVec3ToAdd.y + y, IntVec3ToAdd.z + z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec3 IntVec3::operator+(const IntVec3& IntVec3ToAdd) const
{
	return IntVec3(IntVec3ToAdd.x + x, IntVec3ToAdd.y + y, IntVec3ToAdd.z + z);
}

IntVec3 IntVec3::operator-(const IntVec3& other) const
{
	return IntVec3(x - other.x, y - other.y, z - other.z);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec3::SetFromText(const char* text)
{
	Strings stringVec2 = SplitStringOnDelimiter(text, ',');
	x = std::stoi(stringVec2[0]);
	y = std::stoi(stringVec2[1]);
	z = std::stoi(stringVec2[2]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void IntVec3::SetFromTextWithDelimiter(const char* text, char Delimiter)
{
	Strings stringVec3 = SplitStringOnDelimiter(text, Delimiter);
	x = std::stoi(stringVec3[0]);
	stringVec3[1] == "" ? y = 0 : y = std::stoi(stringVec3[1]);
	z = std::stoi(stringVec3[2]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------