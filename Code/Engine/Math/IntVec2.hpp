#pragma once
#include "Engine/Math/Vec2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vec2;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct IntVec2
{
public:
	int x = 0;
	int y = 0;

	~IntVec2() {};											
	IntVec2() {};											
	IntVec2(const IntVec2& copyFrom);							
	explicit IntVec2(int initialX, int initialY);
	explicit IntVec2(Vec2 XY);

//Accessors(const methods)

	float			GetLength() const;
	int				GetTaxicabLength() const;
	int				GetLengthSquared() const;
	float			GetOrientationRadians() const;
	float			GetOrientationDegrees() const;
	IntVec2 const	GetRotated90Degrees() const;
	IntVec2 const	GetRotatedMinus90Degrees() const;

//Mutators(non-const methods)
	void			Rotate90Degrees();
	void			RotateMinus90Degrees();

	void			SetFromText(char const* text);
	void			SetFromTextWithDelimiter(const char* text, char Delimiter);

	// Operators (const)
	bool		    operator==(const IntVec2& compare) const;		// vec2 == vec2
	bool		    operator!=(const IntVec2& compare) const;		// vec2 != vec2
	bool			operator<(const IntVec2& other) const;
	const IntVec2	operator+(const IntVec2& vecToAdd) const;		// vec2 + vec2
	const IntVec2	operator-(const IntVec2& vecToSubtract) const;	// vec2 - vec2
	const IntVec2	operator-() const;								// -vec2, i.e. "unary negation"
	const IntVec2	operator*(float uniformScale) const;			// vec2 * float
	const IntVec2	operator*(const IntVec2& vecToMultiply) const;	// vec2 * vec2
	const IntVec2	operator/(float inverseScale) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		   operator+=(const IntVec2& vecToAdd);				// vec2 += vec2
	void		   operator-=(const IntVec2& vecToSubtract);		// vec2 -= vec2
	void		   operator*=(const float uniformScale);			// vec2 *= float
	void		   operator/=(const float uniformDivisor);			// vec2 /= float
	void		   operator=(const IntVec2& copyFrom);				// vec2 = vec2
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------