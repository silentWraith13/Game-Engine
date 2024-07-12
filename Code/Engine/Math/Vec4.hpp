#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vec3;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vec4 
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	Vec4() = default;
	Vec4(const Vec4& copyFrom);
	Vec4(Vec3 const& copyFrom);
	Vec4(Vec3 const& copyFrom, float const& wValue);
	explicit Vec4(float initialX, float initialY, float initialZ, float initialW);

	const Vec4	operator+(const Vec4& vecToAdd) const { return Vec4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w); }
	const Vec4	operator-(const Vec4& vecToSubtract) const { return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w); }
	const Vec4	operator-() const { return Vec4(-x, -y, -z, -w); }
	const Vec4	operator*(float uniformScale) const { return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale); }
	const Vec4	operator*(const Vec4& vecToMultiply) const { return Vec4(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z, w * vecToMultiply.w); }
	const Vec4	operator/(float inverseScale) const { return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale); }

	void		operator+=(const Vec4& vecToAdd) { x += vecToAdd.x; y += vecToAdd.y; z += vecToAdd.z; w += vecToAdd.w; }
	void		operator-=(const Vec4& vecToSubtract) { x -= vecToSubtract.x; y -= vecToSubtract.y; z -= vecToSubtract.z; w -= vecToSubtract.w; }
	void		operator*=(float uniformScale) { x *= uniformScale; y *= uniformScale; z *= uniformScale; w *= uniformScale; }
	void		operator/=(float uniformDivisor) { x /= uniformDivisor; y /= uniformDivisor; z /= uniformDivisor; w /= uniformDivisor; }
	void		operator=(const Vec4& copyFrom) { x = copyFrom.x; y = copyFrom.y; z = copyFrom.z; w = copyFrom.w; }
	bool		operator==(const Vec4& compare) const { return (x == compare.x) && (y == compare.y) && (z == compare.z) && (w == compare.w); }
	bool		operator!=(const Vec4& compare) const { return !(*this == compare); }
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------