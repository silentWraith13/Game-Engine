#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vec3;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	static IntVec3 const  ZERO;
	static IntVec3 const  ONE;
	static IntVec3 const  INVALID;

	~IntVec3() {}
	IntVec3() {}
	IntVec3(IntVec3 const& copyFrom);
	IntVec3(Vec3 const& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	//Accessors
	float GetLength() const;
	int	  GetLengthSquared() const;
	int	  GetTaxicabLength() const;
	IntVec3 const GetRotated90Degrees() const;
	IntVec3 const GetRotatedMinus90Degrees() const;
	float GetOrientationDegrees() const;
	float GetOrientationRadians() const;

	//Mutators
	void Rotate90Degrees();
	void RotateMinus90Degrees();

	bool	operator==(const IntVec3& compare) const;
	bool	operator!=(const IntVec3& compare) const;
	IntVec3	operator+(const IntVec3& IntVec3ToAdd);
	IntVec3 operator+(const IntVec3& IntVec3ToAdd) const;
	IntVec3 operator-(const IntVec3& other) const;

	void SetFromText(const char* text);
	void SetFromTextWithDelimiter(const char* text, char Delimiter);
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
