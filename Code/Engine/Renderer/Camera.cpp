#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight)
{
	bounds.m_mins = bottomLeft;
	bounds.m_maxs = topRight;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near /*= 0.0f*/, float far /*= 1.0f*/)
{
	m_mode = eMode_Orthographic;

	m_orthographicBottomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = near;
	m_orthographicFar = far;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	m_mode = eMode_Perspective;
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = near;
	m_perspectiveFar = far;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::Translate2D(Vec2 const& translation)
{
	(void)translation;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoBottomLeft() const
{
	return bounds.m_mins;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoTopRight() const
{
	return bounds.m_maxs;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Camera::GetCameraCenter() const
{
	Vec2 currentCenter = bounds.GetCenter();
	return currentCenter;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Camera::GetForwardVector() const
{
	return m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D().GetNormalized();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetCameraCenter(Vec2 const& newCenter)
{
	bounds.SetCenter(newCenter);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetTransform(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 Camera::GetCameraAABB2D() const
{
	return bounds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 Camera::GetViewport() const
{
	return m_viewport;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetOrthographicMatrix() const
{
	return Mat44::CreateOrthoProjection(m_orthographicBottomLeft.x, m_orthographicTopRight.x,m_orthographicBottomLeft.y, m_orthographicTopRight.y,m_orthographicNear, m_orthographicFar	);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 projectionMatrix;

	if (m_mode == eMode_Orthographic)
	{
		projectionMatrix = GetOrthographicMatrix();
	}
	
	if(m_mode == eMode_Perspective)
	{
		projectionMatrix = GetPerspectiveMatrix();
	}

 	Mat44 renderMatrix = GetRenderMatrix();
 
 	projectionMatrix.Append(renderMatrix);

	return projectionMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetRenderMatrix() const
{
	Mat44 renderMatrix;
	renderMatrix.SetIJK3D(m_renderIBasis, m_renderJBasis, m_renderKBasis);
	return renderMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetViewMatrix() const
{
	Mat44 viewMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	viewMatrix.SetTranslation3D(m_position);
	return viewMatrix.GetOrthonormalInverse();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetModelMatrix() const
{
	Mat44 rotation = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	rotation.SetTranslation3D(m_position);
	Mat44 modelMatrix = rotation;
	return modelMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Camera::GetMouseWorldDirection(const Vec2& cursorScreenPosition, const Vec2& screenDimensions) 
{
	float aspectRatio = screenDimensions.x / screenDimensions.y;
	float angle = tanf(ConvertDegreesToRadians(m_perspectiveFOV * 0.5f));
	float dx = cursorScreenPosition.x - screenDimensions.x * 0.5f;
	float dy = cursorScreenPosition.y - screenDimensions.y * 0.5f;

	dx = RangeMapClamped(dx, -screenDimensions.x * 0.5f, screenDimensions.x * 0.5f, -1.0f, 1.0f);
	dy = RangeMapClamped(dy, -screenDimensions.y * 0.5f, screenDimensions.y * 0.5f, -1.0f, 1.0f);

	Vec3 X = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 Y = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 Z = Vec3(0.0f, 0.0f, 1.0f);

	Vec3 A = m_perspectiveNear * X;
	Vec3 B = -dx * aspectRatio * m_perspectiveNear * angle * Y;
	Vec3 C = -dy * m_perspectiveNear * angle * Z;

	Vec3 viewSpacePoint = A + B + C;

	Mat44 cameraModelMatrix = GetModelMatrix();
	Vec3 worldSpacePoint = cameraModelMatrix.TransformPosition3D(viewSpacePoint);

	Vec3 direction = (worldSpacePoint - m_position).GetNormalized();

	return direction;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetStereoscopicView(Mat44 stereoScopicProjectionMatrix, Mat44 viewToRender)
{
	(void)(stereoScopicProjectionMatrix);
	m_mode = eMode_Stereoscope;
	m_viewToRenderMatrix = viewToRender;
	m_stereoScopicCameraMatrix = GetPerspectiveMatrix();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetStereoscopicView(float aspect, float fov, float nearz, float farz, float left, float right)
{
	m_mode = eMode_Stereoscope;
	m_stereoScopicCameraMatrix = Mat44::CreateStereoscopicProjectionMatrix(fov, aspect, nearz, farz, left, right);
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetStereoScopicMatrix() const
{
	return m_stereoScopicCameraMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetViewToRenderTransform(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_viewToRenderMatrix = Mat44(iBasis, jBasis, kBasis, Vec3());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetViewToRenderMatrix() const
{
	return m_viewToRenderMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetViewMatrixAtOrigin() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D(Vec3());
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix.GetOrthonormalInverse();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Camera::SetAndGetLookAtMatrix(const Vec3& positionToLookAt)
{
	Vec3 iBasis, jBasis, kBasis;
	iBasis = (m_position - positionToLookAt).GetNormalized();
	Vec3 worldZBasis = Vec3(0.0f, 0.0, 1.0f);
	jBasis = CrossProduct3D(worldZBasis, iBasis);
	kBasis = CrossProduct3D(iBasis, jBasis);
	m_lookAtMatrix = Mat44(iBasis, kBasis, jBasis, Vec3());
	return m_lookAtMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetColorTarget(Texture* tex)
{
	m_colorTarget = tex;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::SetDepthTarget(Texture* tex)
{
	m_depthTarget = tex;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget()
{
	return m_colorTarget;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Camera::GetDepthTarget()
{
	return m_depthTarget;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Camera::DestroyTextures()
{
	if (m_colorTarget)
	{
		m_owner->DestroyTexture(m_colorTarget);
	}
	if (m_depthTarget)
	{
		m_owner->DestroyTexture(m_depthTarget);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------