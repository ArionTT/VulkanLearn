#include "Camera.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"

DEFINITE_CLASS_RTTI(Camera, BaseComponent);

bool Camera::Init(const CameraInfo& info, const std::shared_ptr<Camera>& pCamera)
{
	if (!BaseComponent::Init(pCamera))
		return false;

	SetCameraInfo(info);

	return true;
}

std::shared_ptr<Camera> Camera::Create(const CameraInfo& info)
{
	std::shared_ptr<Camera> pCamera = std::make_shared<Camera>();
	if (pCamera.get() && pCamera->Init(info, pCamera))
		return pCamera;
	return nullptr;
}

void Camera::Update()
{
}

void Camera::OnPreRender()
{
	UpdateCameraPosition();
	UpdateViewMatrix();
	UpdateProjMatrix();
}

void Camera::UpdateViewMatrix()
{
	if (m_pObject.expired())
		return;

	Matrix4d matrix = m_pObject.lock()->GetCachedWorldTransform();
	UniformData::GetInstance()->GetPerFrameUniforms()->SetViewCoordinateSystem(matrix);

	matrix.Inverse();
	UniformData::GetInstance()->GetPerFrameUniforms()->SetViewMatrix(matrix);

	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraDirection(m_pObject.lock()->GetCachedWorldTransform()[2].xyz().Negative());
}

void Camera::UpdateProjMatrix()
{
	if (!m_projDirty)
		return;

	double tanFOV2 = std::tan(m_cameraInfo.fov / 2.0);

	double A = 0;
	double B = m_cameraInfo.near;

	Matrix4d proj;

	double height = 2.0 * tanFOV2 * m_cameraInfo.near;
	double width = m_cameraInfo.aspect * height;

	// Prepare jitter offset
	//if (m_cameraInfo.jitterOffset != 0)
	{
		proj.x0 = 2.0 * m_cameraInfo.near / width;	//	2 * n / (right - left)

		proj.y1 = -2.0 * m_cameraInfo.near / height;
		// 1). x2 = ((right + jitter_offset_near_plane) + (left + jitter_offset_near_plane)) / ((right + jitter_offset_near_plane) - (left + jitter_offset_near_plane))
		// 2). x2 = (2 * jitter_offset_near_plane) / near_plane_width
		// 3). jitter_offset_near_plane = jitter_offset / window_width * near_plane_width
		// 4). x2 = 2 * jitter_offset / window_width
		// 5). jitter_offset = jitter * window_width
		// 6). x2 = 2 * jitter
		proj.z0 = 2.0 * m_cameraInfo.jitterOffset.x;
		proj.z1 = 2.0 * m_cameraInfo.jitterOffset.y;
		proj.z2 = A;
		proj.z3 = -1.0;

		proj.w2 = B;
		proj.w3 = 0.0;

		UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraJitterOffset(m_cameraInfo.jitterOffset);
	}

	UniformData::GetInstance()->GetPerFrameUniforms()->SetEyeSpaceSize({ width, height });
	UniformData::GetInstance()->GetPerFrameUniforms()->SetNearFarAB({ m_cameraInfo.near, m_cameraInfo.far, A, B });
	UniformData::GetInstance()->GetGlobalUniforms()->SetProjectionMatrix(proj.DoublePrecision());

	m_projDirty = false;
}

void Camera::UpdateCameraPosition()
{
	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraPosition(GetBaseObject()->GetCachedWorldPosition());
}

void Camera::SetFOV(double new_fov)
{
	m_cameraInfo.fov = new_fov; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetAspect(double new_aspect)
{ 
	m_cameraInfo.aspect = new_aspect; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetNearPlane(double new_near_plane)
{ 
	m_cameraInfo.near = new_near_plane; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetJitterOffset(Vector2d jitterOffset)
{
	m_cameraInfo.jitterOffset = jitterOffset;
	m_cameraInfo.jitterOffset.x /= 1440.0;
	m_cameraInfo.jitterOffset.y /= 1024.0;
	m_projDirty = true;
}

void Camera::SetCameraInfo(const CameraInfo& info) 
{ 
	m_cameraInfo = info; 
	UpdateCameraSupplementInfo();

	m_projDirty = true; 
}

void Camera::UpdateCameraSupplementInfo()
{
	double tanFOV2 = std::tan(m_cameraInfo.fov / 2.0);

	//update horizontal fov
	//tan(fovh/2) = asp*tan(fov/2)       1)
	//fovh = 2*arctan(asp*tan(fov/2))    2)
	m_fovH = std::atan(m_cameraInfo.aspect * tanFOV2) * 2.0;
}