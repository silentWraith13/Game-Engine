#pragma once

struct Vec2;
struct Vec3;
struct Vec4;

struct Mat44
{
	enum{Ix, Iy, Iz, Iw,  Jx,Jy,Jz,Jw,  Kx, Ky, Kz, Kw,	Tx,  Ty,Tz, Tw};
	float m_values[16];

	Mat44() ;
	explicit Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D);
	explicit Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D);
	explicit Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	explicit Mat44(float const* sixteenValuesBasisMajor);

	static Mat44 const CreateTranslation2D(Vec2 const& translationXY);
	static Mat44 const CreateTranslation3D(Vec3 const& translationXYZ);
	static Mat44 const CreateUniformScale2D(float uniformScaleXY);
	static Mat44 const CreateUniformScale3D(float uniformScaleXYZ);
	static Mat44 const CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY);
	static Mat44 const CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ);
	static Mat44 const CreateZRotationDegrees(float rotationDegreesAboutZ);
	static Mat44 const CreateYRotationDegrees(float rotationDegreesAboutY);
	static Mat44 const CreateXRotationDegrees(float rotationDegreesAboutX);
	static Mat44 const CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	static Mat44 const CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar);
	static Mat44 const CreateStereoscopicProjectionMatrix(float fovYDegrees, float aspect, float zNear, float zFar, float left, float right);
	static Mat44 const CreateStereoscopicProjectionMatrix(float eyeDistance, float left, float right, float up, float down, float zNear, float zFar);

	Vec2 const TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const;  //assumes z=0, w=0
	Vec3 const TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const;  // w=0
	Vec2 const TransformPosition2D(Vec2 const& positionXY) const; //assumes z=0, w=1
	Vec3 const TransformPosition3D(Vec3 const& positionXYZ) const; //assumes w=1
	Vec4 const TransformHomogeneous3D(Vec4 const& homogenousPoint3D) const;

 	float*			GetAsFloatArray();
 	float const*	GetAsFloatArray() const;
	Vec2 const		GetIBasis2D() const;
	Vec2 const		GetJBasis2D() const;
	Vec2 const		GetTranslation2D() const;
	Vec3 const		GetIBasis3D() const;
	Vec3 const		GetJBasis3D() const;
	Vec3 const		GetKBasis3D() const;
	Vec3 const		GetTranslation3D() const;
	Vec4 const		GetIBasis4D() const;
	Vec4 const		GetJBasis4D() const;
	Vec4 const		GetKBasis4D() const;
	Vec4 const		GetTranslation4D() const;
	Mat44 const		GetOrthonormalInverse() const;

	void SetTranslation2D(Vec2 const& translationXY);		//Sets TranslationZ = 0, translationW = 1;
	void SetTranslation3D(Vec3 const& translationXYZ);	// Sets TranslationW = 1;
	void SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D);	//Sets z=0,w=0 for i,j,t:does not modify k;
	void SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY);		//sets z=0,w=0 for i,j,t:does not modify k
	void SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D);		//sets w=0 for i,j,k
	void SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ);		//sets w=0 for i,j,k,t
	void SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	void Transpose();			//Swaps columns with rows.
	void Orthonormalize_XFwd_YLeft_ZUp();	
	Mat44                 GetNonOrthonormalInverse(); // For Non-Affine Matrices
	void				  MapMatrixTo2DArray(Mat44 copyFrom, float copyTo[4][4]); // For Non-Affine Matrices
	void				  Map2DArrayToMatrix(float copyFrom[4][4], Mat44& copyTo); // For Non-Affine Matrices

	void Append(Mat44 const& appendThis);
	void AppendZRotation(float degreesRotationAboutZ);		//same as appending (*= in column notation) a z-rotation matrix
	void AppendYRotation(float degreesRotationAboutY);		//same as appending (*= in column notation) a y-rotation matrix
	void AppendXRotation(float degreesRotationAboutX);			 //same as appending (*= in column notation) a x-rotation matrix
	void AppendTranslation2D(Vec2 const& translationXY);		//same as appending (*= in column notation) a translation matrix
	void AppendTranslation3D(Vec3 const& translationXYZ);		//same as appending (*= in column notation) a translation matrix 
	void AppendScaleUniform2D(float uniformScaleXY);	//k and t basis should remain unaffected
	void AppendScaleUniform3D(float uniformScaleXYZ);	//translation should remain unaffected
	void AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY);	//k and t basis should remain unaffected
	void AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ);	//translation should remain unaffected

	Mat44 const		GetInversedMatrix() const;
	float			Determinant3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i) const;
	float			Determinant() const;
	void				  SwapValues(float& valuea, float& valueb);
};

void GetCofactor(float A[4][4], float temp[4][4], int p, int q, int n);
float Determinant(float A[4][4], int n);
void Adjoint(float A[4][4], float adj[4][4]);
bool Inverse(float A[4][4], float inverse[4][4]);