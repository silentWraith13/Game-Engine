#pragma once

struct IntRange
{
public:
	IntRange() = default;
	IntRange(int min, int max);

	void operator=(const IntRange& other);
	bool operator==(const IntRange& other) const;
	bool operator!=(const IntRange& other) const;
	bool IsOnRange(int value) const;
	bool IsOverlappingWith(const IntRange& other) const;

public:
	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;

	int m_min;
	int m_max;
};


