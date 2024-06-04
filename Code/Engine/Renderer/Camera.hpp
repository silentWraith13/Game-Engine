#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"


class Texture;
class Renderer;

class Camera 
{

public:
	enum Mode
	{
		eMode_Orthographic,
		eMode_Perspective,
		eMode_Stereoscope,
		eMode_Count
	};


	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight);
	void SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f);
	void SetPerspectiveView(float aspect, float fov, float near, float far);
	void SetStereoscopicView(Mat44 stereoScopicProjectionMatrix, Mat44 viewToRender);
	void SetStereoscopicView(float aspect, float fov, float, float, float left, float right);
	void Translate2D(Vec2 const& translation);
	void SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	void SetCameraCenter(Vec2 const& newCenter);
	void SetTransform(const Vec3& position, const EulerAngles& orientation);
	void				SetViewToRenderTransform(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	Vec2 GetCameraCenter() const;

	Vec3 GetForwardVector() const;
	
	AABB2 GetCameraAABB2D() const;
	AABB2 GetViewport() const;

	Mat44 GetOrthographicMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;
	Mat44 GetRenderMatrix() const;
	Mat44 GetStereoScopicMatrix() const;
	Mat44 GetViewMatrix() const;
	Mat44 GetModelMatrix() const;
	Vec3 GetMouseWorldDirection(const Vec2& cursorScreenPosition, const Vec2& screenDimensions);
	Mat44				GetViewToRenderMatrix() const;
	Mat44				GetViewMatrixAtOrigin() const;
	Mat44				SetAndGetLookAtMatrix(const Vec3& positionToLookAt);
	void				SetColorTarget(Texture* tex);
	void				SetDepthTarget(Texture* tex);
	Texture* GetColorTarget();
	Texture* GetDepthTarget();
	void				DestroyTextures();

public:
	AABB2 bounds;
	AABB2 m_viewport;
	
	Mode m_mode = eMode_Orthographic;

	Vec2 m_orthographicBottomLeft;
	Vec2 m_orthographicTopRight;
	
	float m_orthographicNear;
	float m_orthographicFar;
	float m_perspectiveAspect;
	float m_perspectiveFOV;
	float m_perspectiveNear;
	float m_perspectiveFar;

	Vec3 m_position;
	Vec3 m_renderIBasis = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 m_renderJBasis = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 m_renderKBasis = Vec3(0.0f, 0.0f, 1.0f);
	Mat44 m_viewToRenderMatrix;
	EulerAngles m_orientation;
	Mat44				m_perspectiveCameraMatrix;
	Mat44				m_orthogrphicCameraMatrix;
	Mat44				m_stereoScopicCameraMatrix;
	Mat44				m_stereoScopicEyeMatrix;
	Mat44				m_lookAtMatrix;
	Texture* m_colorTarget = nullptr;
	Texture* m_depthTarget = nullptr;
	Renderer* m_owner;
};
