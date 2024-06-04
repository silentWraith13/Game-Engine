#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct FloatRange
{
public:
	FloatRange() = default;
	FloatRange(float min, float max);

	void operator=(const FloatRange& other);
	bool operator==(const FloatRange& other) const;
	bool operator!=(const FloatRange& other) const;
	bool IsOnRange(float value) const;
	bool IsOverlappingWith(const FloatRange& other) const;
	void SetFromText(char const* text);
	FloatRange GetOverlappingRange(FloatRange const& compareTo) const;

public:
	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;

	float m_min;
	float m_max;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------

