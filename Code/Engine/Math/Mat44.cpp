#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44::Mat44()
{
	m_values[Ix] = 1.0f;
	m_values[Iy] = 0.0f;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;

	m_values[Jx] = 0.0f;
	m_values[Jy] = 1.0f;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;

	m_values[Kx] = 0.0f;
	m_values[Ky] = 0.0f;
	m_values[Kz] = 1.0f;
	m_values[Kw] = 0.0f;

	m_values[Tx] = 0.0f;
	m_values[Ty] = 0.0f;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Kx] = 0.f;
	m_values[Ky] = 0.f;
	m_values[Kz] = 1.f;
	m_values[Kw] = 0.f;

	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	m_values[Ix] = sixteenValuesBasisMajor[0];
	m_values[Iy] = sixteenValuesBasisMajor[1];
	m_values[Iz] = sixteenValuesBasisMajor[2];
	m_values[Iw] = sixteenValuesBasisMajor[3];

	m_values[Jx] = sixteenValuesBasisMajor[4];
	m_values[Jy] = sixteenValuesBasisMajor[5];
	m_values[Jz] = sixteenValuesBasisMajor[6];
	m_values[Jw] = sixteenValuesBasisMajor[7];

	m_values[Kx] = sixteenValuesBasisMajor[8];
	m_values[Ky] = sixteenValuesBasisMajor[9];
	m_values[Kz] = sixteenValuesBasisMajor[10];
	m_values[Kw] = sixteenValuesBasisMajor[11];

	m_values[Tx] = sixteenValuesBasisMajor[12];
	m_values[Ty] = sixteenValuesBasisMajor[13];
	m_values[Tz] = sixteenValuesBasisMajor[14];
	m_values[Tw] = sixteenValuesBasisMajor[15];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	Mat44 translation;
	translation.m_values[Tx] = translationXY.x;
	translation.m_values[Ty] = translationXY.y;
	return translation;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------

Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translation;
	translation.m_values[Tx] = translationXYZ.x;
	translation.m_values[Ty] = translationXYZ.y;
	translation.m_values[Tz] = translationXYZ.z;
	return translation;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	Mat44 scale2D;
	scale2D.m_values[Ix] *= uniformScaleXY;
	scale2D.m_values[Jy] *=uniformScaleXY;
	return scale2D;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	Mat44 scale3D;
	scale3D.m_values[Ix] *= uniformScaleXYZ;
	scale3D.m_values[Jy] *= uniformScaleXYZ;
	scale3D.m_values[Kz] *= uniformScaleXYZ;
	return scale3D;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 mat;
	mat.m_values[Ix] *= nonUniformScaleXY.x;
	mat.m_values[Jy] *= nonUniformScaleXY.y;
	return mat;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scale3D;
	scale3D.m_values[Ix] *= nonUniformScaleXYZ.x;
	scale3D.m_values[Jy] *= nonUniformScaleXYZ.y;
	scale3D.m_values[Kz] *= nonUniformScaleXYZ.z;
	return scale3D;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	Mat44 rotZ;
	float c = CosDegrees(rotationDegreesAboutZ);
	float s = SinDegrees(rotationDegreesAboutZ);
	rotZ.m_values[Ix] = c;
	rotZ.m_values[Iy] = s;
	rotZ.m_values[Jx] = -s;
	rotZ.m_values[Jy] = c;
	return rotZ;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	Mat44 rotY;
	float c = CosDegrees(rotationDegreesAboutY);
	float s = SinDegrees(rotationDegreesAboutY);
	rotY.m_values[Ix] = c;
	rotY.m_values[Kx] = s;
	rotY.m_values[Iz] = -s;
	rotY.m_values[Kz] = c;
	return rotY;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	Mat44 rotX;
	float c = CosDegrees(rotationDegreesAboutX);
	float s = SinDegrees(rotationDegreesAboutX);
	rotX.m_values[Jy] = c;
	rotX.m_values[Jz] = s;
	rotX.m_values[Ky] = -s;
	rotX.m_values[Kz] = c;
	return rotX;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
 	Mat44 projection;

	float rml = right - left;
	float tmb = top - bottom;
	float fmn = zFar - zNear;

	projection.m_values[Ix] = 2.0f / rml;
	projection.m_values[Jy] = 2.0f / tmb;
	projection.m_values[Kz] = 1.0f / fmn;
	projection.m_values[Tx] = -(right + left) / rml;
	projection.m_values[Ty] = -(top + bottom) / tmb;
	projection.m_values[Tz] = -(zNear) / fmn;
	projection.m_values[Tw] = 1.0f;

	return projection;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 projectionMatrix;
	float scaleY = CosDegrees(fovYDegrees * 0.5f) / SinDegrees(fovYDegrees * 0.5f);
	float scaleX = scaleY / aspect;
	float scaleZ = zFar / (zFar - zNear);
	float translateZ = ( zNear * zFar) / (zNear - zFar);

	projectionMatrix.m_values[Ix] = scaleX;
	projectionMatrix.m_values[Jy] = scaleY;
	projectionMatrix.m_values[Kz] = scaleZ;
	projectionMatrix.m_values[Kw] = 1.0f;
	projectionMatrix.m_values[Tz] = translateZ;
	projectionMatrix.m_values[Tw] = 0.0f;

	return projectionMatrix;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	return Vec2( (m_values[Ix] * vectorQuantityXY.x) + (m_values[Jx] * vectorQuantityXY.y), (m_values[Iy] * vectorQuantityXY.x) + (m_values[Jy] * vectorQuantityXY.y) );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	float Px = (m_values[Ix] * vectorQuantityXYZ.x) + (m_values[Jx] * vectorQuantityXYZ.y) + (m_values[Kx] * vectorQuantityXYZ.z);
	float Py = (m_values[Iy] * vectorQuantityXYZ.x) + (m_values[Jy] * vectorQuantityXYZ.y) + (m_values[Ky] * vectorQuantityXYZ.z);
	float Pz = (m_values[Iz] * vectorQuantityXYZ.x) + (m_values[Jz] * vectorQuantityXYZ.y) + (m_values[Kz] * vectorQuantityXYZ.z);
	return Vec3(Px, Py, Pz);
}
	
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	return Vec2( (  (m_values[Ix] * positionXY.x) + (m_values[Jx] * positionXY.y) + m_values[Tx] ), ( (m_values[Iy] * positionXY.x) + (m_values[Jy] * positionXY.y) + m_values[Ty]) );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::TransformPosition3D(Vec3 const& positionXYZ) const
{
	return Vec3(((m_values[Ix] * positionXYZ.x) + (m_values[Jx] * positionXYZ.y) + ( m_values[Kx] * positionXYZ.z) + m_values[Tx]), ((m_values[Iy] * positionXYZ.x) + (m_values[Jy] * positionXYZ.y ) + (m_values[Ky] * positionXYZ.z) + m_values[Ty]), ( (m_values[Iz] * positionXYZ.x) + (m_values[Jz] * positionXYZ.y) + (m_values[Kz] * positionXYZ.z) + m_values[Tz]) );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogenousPoint3D) const
{
	float Px = m_values[Ix] * homogenousPoint3D.x + m_values[Jx] * homogenousPoint3D.y + m_values[Kx] * homogenousPoint3D.z + m_values[Tx] * homogenousPoint3D.w;
	float Py = m_values[Iy] * homogenousPoint3D.x + m_values[Jy] * homogenousPoint3D.y + m_values[Ky] * homogenousPoint3D.z + m_values[Ty] * homogenousPoint3D.w;
	float Pz = m_values[Iz] * homogenousPoint3D.x + m_values[Jz] * homogenousPoint3D.y + m_values[Kz] * homogenousPoint3D.z + m_values[Tz] * homogenousPoint3D.w;
	float Pw = m_values[Iw] * homogenousPoint3D.x + m_values[Jw] * homogenousPoint3D.y + m_values[Kw] * homogenousPoint3D.z + m_values[Tw] * homogenousPoint3D.w;
	return Vec4(Px, Py, Pz, Pw);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
 float* Mat44::GetAsFloatArray()
 {
	 return m_values;
 }
 
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 float const* Mat44::GetAsFloatArray() const
 {
	 return &m_values[0];
 }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix],m_values[Iy]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx],m_values[Jy]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix],m_values[Iy],m_values[Iz]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix],m_values[Iy],m_values[Iz],m_values[Iw]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 roatationIsolatedAndTransposedMatrix;
	
	// Isolate i,j,k = Rotation  only matrix.
	roatationIsolatedAndTransposedMatrix.SetIJK3D(GetIBasis3D(), GetJBasis3D(), GetKBasis3D());
	
	//Transpose it. (inverse)R
	roatationIsolatedAndTransposedMatrix.Transpose();
	
	//Isolate 'T'
	Mat44 translationOnlyMatrix;
	Vec3 translation = GetTranslation3D();
	
	//Negate T
	translationOnlyMatrix.SetTranslation3D(-1.f * translationOnlyMatrix.TransformPosition3D(translation));

	//Recombine the isolated Rotated, transposed "Rotation" matrix and negated "Translation" matrix.
	roatationIsolatedAndTransposedMatrix.Append(translationOnlyMatrix);

	return roatationIsolatedAndTransposedMatrix;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::Transpose()
{
	float temp;

	temp = m_values[Jx];  m_values[Jx] = m_values[Iy];  m_values[Iy] = temp;
	
	temp = m_values[Kx];  m_values[Kx] = m_values[Iz];  m_values[Iz] = temp;
	
	temp = m_values[Tx];  m_values[Tx] = m_values[Iw];  m_values[Iw] = temp;

	temp = m_values[Ky];  m_values[Ky] = m_values[Jz];  m_values[Jz] = temp;
	
	temp = m_values[Ty];  m_values[Ty] = m_values[Jw];  m_values[Jw] = temp;

	temp = m_values[Tz];  m_values[Tz] = m_values[Kw];  m_values[Kw] = temp;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::Orthonormalize_XFwd_YLeft_ZUp()
{	
	// Step 1: Normalize the i-basis.
	Vec3 iBasis(m_values[Ix], m_values[Iy], m_values[Iz]);
	iBasis.Normalize();

	// Step 2: Calculate the K-basis.
	Vec3 jBasis(m_values[Jx], m_values[Jy], m_values[Jz]);
	Vec3 kBasis = CrossProduct3D(iBasis, jBasis);
	kBasis.Normalize();

	// Step 3: Calculate the length of K in i's direction.
	float kInIDirection = DotProduct3D(kBasis, iBasis);

	// Step 4: Calculate Ki vector = length of K in i's direction * iBasis.Normalized.
	Vec3 kiVector = iBasis * kInIDirection;

	// Step 5: Subtract i parts from K basis.
	kBasis = kBasis - kiVector;

	// Step 6: Normalize K.
	kBasis.Normalize();

	// Step 7: Calculate Ji vector = (DotProduct3D(J Basis vector, Normalized I basis) * Ibasis.Normalized()).
	Vec3 jiVector = iBasis * DotProduct3D(jBasis, iBasis);

	// Step 8: Subtract i parts from J basis.
	jBasis = jBasis - jiVector;

	// Step 9: Find Jk = (DotProduct3D(J Basis vector, Normalized K basis)*Kbasis.Normalized()).
	Vec3 jkVector = kBasis * DotProduct3D(jBasis, kBasis);

	// Step 10: Subtract K parts from the J vector.
	jBasis = jBasis - jkVector;

	// Step 11: Normalize J.
	jBasis.Normalize();

	// Update the matrix with orthonormalized basis vectors.
	m_values[Ix] = iBasis.x;
	m_values[Iy] = iBasis.y;
	m_values[Iz] = iBasis.z;
	m_values[Jx] = jBasis.x;
	m_values[Jy] = jBasis.y;
	m_values[Jz] = jBasis.z;
	m_values[Kx] = kBasis.x;
	m_values[Ky] = kBasis.y;
	m_values[Kz] = kBasis.z;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 copyOfThis = *this;
	float const* left = &copyOfThis.m_values[0];
	float const* right = &appendThis.m_values[0];

	m_values[Ix] = (left[Ix] * right[Ix]) + (left[Jx] * right[Iy]) + (left[Kx] * right[Iz]) + (left[Tx] * right[Iw]);
	m_values[Iy] = (left[Iy] * right[Ix]) + (left[Jy] * right[Iy]) + (left[Ky] * right[Iz]) + (left[Ty] * right[Iw]);
	m_values[Iz] = (left[Iz] * right[Ix]) + (left[Jz] * right[Iy]) + (left[Kz] * right[Iz]) + (left[Tz] * right[Iw]);
	m_values[Iw] = (left[Iw] * right[Ix]) + (left[Jw] * right[Iy]) + (left[Kw] * right[Iz]) + (left[Tw] * right[Iw]);

	m_values[Jx] = (left[Ix] * right[Jx]) + (left[Jx] * right[Jy]) + (left[Kx] * right[Jz]) + (left[Tx] * right[Jw]);
	m_values[Jy] = (left[Iy] * right[Jx]) + (left[Jy] * right[Jy]) + (left[Ky] * right[Jz]) + (left[Ty] * right[Jw]);
	m_values[Jz] = (left[Iz] * right[Jx]) + (left[Jz] * right[Jy]) + (left[Kz] * right[Jz]) + (left[Tz] * right[Jw]);
	m_values[Jw] = (left[Iw] * right[Jx]) + (left[Jw] * right[Jy]) + (left[Kw] * right[Jz]) + (left[Tw] * right[Jw]);

	m_values[Kx] = (left[Ix] * right[Kx]) + (left[Jx] * right[Ky]) + (left[Kx] * right[Kz]) + (left[Tx] * right[Kw]);
	m_values[Ky] = (left[Iy] * right[Kx]) + (left[Jy] * right[Ky]) + (left[Ky] * right[Kz]) + (left[Ty] * right[Kw]);
	m_values[Kz] = (left[Iz] * right[Kx]) + (left[Jz] * right[Ky]) + (left[Kz] * right[Kz]) + (left[Tz] * right[Kw]);
	m_values[Kw] = (left[Iw] * right[Kx]) + (left[Jw] * right[Ky]) + (left[Kw] * right[Kz]) + (left[Tw] * right[Kw]);

	m_values[Tx] = (left[Ix] * right[Tx]) + (left[Jx] * right[Ty]) + (left[Kx] * right[Tz]) + (left[Tx] * right[Tw]);
	m_values[Ty] = (left[Iy] * right[Tx]) + (left[Jy] * right[Ty]) + (left[Ky] * right[Tz]) + (left[Ty] * right[Tw]);
	m_values[Tz] = (left[Iz] * right[Tx]) + (left[Jz] * right[Ty]) + (left[Kz] * right[Tz]) + (left[Tz] * right[Tw]);
	m_values[Tw] = (left[Iw] * right[Tx]) + (left[Jw] * right[Ty]) + (left[Kw] * right[Tz]) + (left[Tw] * right[Tw]);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Mat44 rotZ = Mat44::CreateZRotationDegrees(degreesRotationAboutZ);
	Append(rotZ);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Mat44 rotY = Mat44::CreateYRotationDegrees(degreesRotationAboutY);
	Append(rotY);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Mat44 rotX = Mat44::CreateXRotationDegrees(degreesRotationAboutX);
	Append(rotX);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Mat44 translation = Mat44::CreateTranslation2D(translationXY);
	Append(translation);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translation = Mat44::CreateTranslation3D(translationXYZ);
	Append(translation);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Mat44 scaleUniform = Mat44::CreateUniformScale2D(uniformScaleXY);
	Append(scaleUniform);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Mat44 scaleUniform = Mat44::CreateUniformScale3D(uniformScaleXYZ);
	Append(scaleUniform);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 scaleNonUniform = Mat44::CreateNonUniformScale2D(nonUniformScaleXY);
	Append(scaleNonUniform);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scaleNonUniform = Mat44::CreateNonUniformScale3D(nonUniformScaleXYZ);
	Append(scaleNonUniform);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Mat44::Determinant3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i) const
{
	return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Mat44::Determinant() const
{
	return m_values[0] * Determinant3x3(m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]) -
		m_values[1] * Determinant3x3(m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]) +
		m_values[2] * Determinant3x3(m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]) -
		m_values[3] * Determinant3x3(m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::GetInversedMatrix() const
{
	Mat44 result;
	float det = this->Determinant();
	if (fabs(det) < 1e-6f) 
	{
		
		return *this;
	}
	float invDet = 1.0f / det;
	result.m_values[0] = Determinant3x3(m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]) * invDet;
	result.m_values[1] = -Determinant3x3(m_values[1], m_values[2], m_values[3], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]) * invDet;
	result.m_values[2] = Determinant3x3(m_values[1], m_values[2], m_values[3], m_values[5], m_values[6], m_values[7], m_values[13], m_values[14], m_values[15]) * invDet;
	result.m_values[3] = -Determinant3x3(m_values[1], m_values[2], m_values[3], m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11]) * invDet;
	result.m_values[4] = -Determinant3x3(m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]) * invDet;
	result.m_values[5] = Determinant3x3(m_values[0], m_values[2], m_values[3], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]) * invDet;
	result.m_values[6] = -Determinant3x3(m_values[0], m_values[2], m_values[3], m_values[4], m_values[6], m_values[7], m_values[12], m_values[14], m_values[15]) * invDet;
	result.m_values[7] = Determinant3x3(m_values[0], m_values[2], m_values[3], m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11]) * invDet;
	result.m_values[8] = Determinant3x3(m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]) * invDet;
	result.m_values[9] = -Determinant3x3(m_values[0], m_values[1], m_values[3], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]) * invDet;
	result.m_values[10] = Determinant3x3(m_values[0], m_values[1], m_values[3], m_values[4], m_values[5], m_values[7], m_values[12], m_values[13], m_values[15]) * invDet;
	result.m_values[11] = -Determinant3x3(m_values[0], m_values[1], m_values[3], m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11]) * invDet;
	result.m_values[12] = -Determinant3x3(m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]) * invDet;
	result.m_values[13] = Determinant3x3(m_values[0], m_values[1], m_values[2], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]) * invDet;
	result.m_values[14] = -Determinant3x3(m_values[0], m_values[1], m_values[2], m_values[4], m_values[5], m_values[6], m_values[12], m_values[13], m_values[14]) * invDet;
	result.m_values[15] = Determinant3x3(m_values[0], m_values[1], m_values[2], m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10]) * invDet;
	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

Mat44 const Mat44::CreateStereoscopicProjectionMatrix(float fovYDegrees, float aspect, float zNear, float zFar, float left, float right)
{
	Mat44 returnValue;
	float Sy;
	float Sx;
	float Sz;
	float TZ, TX;
	Sy = (CosDegrees(fovYDegrees * 0.5f) / SinDegrees(fovYDegrees * 0.5f));
	Sz = zFar / (zFar - zNear);
	Sx = Sy / aspect;
	TX = left + right;
	TZ = (zNear * zFar) / (zNear - zFar);
	returnValue.m_values[Ix] = Sx;
	returnValue.m_values[Jy] = Sy;
	returnValue.m_values[Kz] = Sz;
	returnValue.m_values[Kw] = 1.0f;
	//returnValue.m_values[Tx] = TX;
	returnValue.m_values[Tz] = TZ;
	returnValue.m_values[Tw] = 0.0f;
	return returnValue;
}

Mat44 const Mat44::CreateStereoscopicProjectionMatrix(float eyeDistance, float left, float right, float up, float down, float zNear, float zFar)
{
	Mat44 returnValue;
	//float ix, jy, kx, ky, kz, kw, tz;
	//float eyeDistance = right-left/2.0f;
	returnValue.m_values[Ix] = (2 * eyeDistance) / (right - left);
	returnValue.m_values[Jy] = (2 * eyeDistance) / (up - down);
	returnValue.m_values[Kx] = (right + left) / (right - left);
	returnValue.m_values[Ky] = (up + down) / (up - down);
	returnValue.m_values[Kz] = (zNear + zFar) / (zNear - zFar);
	returnValue.m_values[Kw] = -1;
	returnValue.m_values[Tz] = (2 * zNear * zFar) / (zNear - zFar);

	return returnValue;
}

Mat44 Mat44::GetNonOrthonormalInverse()
{
	Mat44 returnInvertedMatrix;
	Mat44 matrix = Mat44(GetIBasis4D(), GetJBasis4D(), GetKBasis4D(), GetTranslation4D());
	matrix.Transpose();
	float MatrixArray[4][4] = { 0 };
	float InverseArray[4][4] = { 0 };
	MapMatrixTo2DArray(matrix, MatrixArray);
	Inverse(MatrixArray, InverseArray);
	Map2DArrayToMatrix(InverseArray, returnInvertedMatrix);
	return returnInvertedMatrix;
}

void Mat44::MapMatrixTo2DArray(Mat44 copyFrom, float copyTo[4][4])
{
	int matrixIndex = 0, row = 0, col = 0;
	for (matrixIndex = 0; matrixIndex < 16; matrixIndex++)
	{
		copyTo[row][col] = copyFrom.m_values[matrixIndex];
		row = row + 1;
		if (row == 4) {
			row = 0;
			col++;
		}
	}
}

Mat44 Invert(Mat44 const& mat)
{
	float inv[16];
	float det;
	float m[16];
	int i;

	for (i = 0; i < 16; ++i) {
		m[i] = (float)mat.m_values[i];
	}

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	det = 1.0f / det;

	Mat44 ret;
	for (i = 0; i < 16; i++) {
		ret.m_values[i] = (float)(inv[i] * det);
	}

	return ret;
}

void Mat44::Map2DArrayToMatrix(float copyFrom[4][4], Mat44& copyTo)
{

	int matrixIndex = 0, row = 0, col = 0;
	for (row = 0; row < 4; row++)
	{
		for (col = 0; col < 4; col++)
		{
			copyTo.m_values[matrixIndex] = copyFrom[row][col];
			matrixIndex++;
		}
	}

}
void GetCofactor(float A[4][4], float temp[4][4], int p, int q, int n)
{
	int i = 0, j = 0;
	for (int row = 0; row < n; row++)
	{
		for (int col = 0; col < n; col++)
		{
			if (row != p && col != q)
			{
				temp[i][j++] = A[row][col];
				if (j == n - 1) {
					j = 0;
					i++;
				}
			}
		}
	}
}

float Determinant(float A[4][4], int n)
{
	float D = 0;

	if (n == 1)
		return A[0][0];

	float temp[4][4];

	int sign = 1;
	for (int f = 0; f < n; f++) {

		GetCofactor(A, temp, 0, f, n);
		D += sign * A[0][f] * Determinant(temp, n - 1);

		sign = -sign;
	}

	return D;
}

void Adjoint(float A[4][4], float adj[4][4])
{
	int sign = 1;
	float temp[4][4];

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			GetCofactor(A, temp, i, j, 4);
			sign = ((i + j) % 2 == 0) ? 1 : -1;
			adj[j][i] = (sign) * (Determinant(temp, 4 - 1));
		}
	}
}

bool Inverse(float A[4][4], float inverse[4][4])
{
	float det = Determinant(A, 4);

	float adj[4][4];
	Adjoint(A, adj);

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			inverse[i][j] = adj[i][j] / float(det);

	return true;
}

void Mat44::SwapValues(float& valuea, float& valueb)
{
	float temp = valuea;
	valuea = valueb;
	valueb = temp;
}