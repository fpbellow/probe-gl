#include "../Headers/Camera.hpp"
Camera::Camera()
	: position({ 1, 1, 1, 1}),
	target({ 0, 0, 0, 0 }),
	up({ 0, 1, 0, 0 })
{
	
	forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
	right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, up));
	UpdateViewMatrix();
}

Camera::Camera(const DirectX::XMVECTOR& pos, const DirectX::XMVECTOR& tar, const DirectX::XMVECTOR& upDir)
	: position(pos), target(tar), up(upDir)
{
	
	forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
	right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, up));
	UpdateViewMatrix();
}

void Camera::Move(const DirectX::XMFLOAT3& direction, float moveSpeed, float deltaTime)
{
	DirectX::XMVECTOR rightScale = DirectX::XMVectorScale(right, direction.x * moveSpeed * deltaTime);
	DirectX::XMVECTOR forwardScale = DirectX::XMVectorScale(forward, direction.z * moveSpeed * deltaTime);
	DirectX::XMVECTOR upScale = DirectX::XMVectorScale(up, direction.y * moveSpeed * deltaTime);

	DirectX::XMVECTOR moveVec = DirectX::XMVectorAdd(DirectX::XMVectorAdd(rightScale, forwardScale), upScale);

	position = DirectX::XMVectorAdd(position, moveVec);
	target = DirectX::XMVectorAdd(target, moveVec);
	UpdateViewMatrix();
}


void Camera::Rotate(float yaw, float pitch)
{
	float pitchLimit = DirectX::XM_PIDIV2 - 0.01f;
	if (pitch > pitchLimit)
		pitch = pitchLimit;
	if (pitch < -pitchLimit)
		pitch = -pitchLimit;

	DirectX::XMFLOAT3 rotation;
	rotation.x = cosf(pitch) * cosf(yaw);
	rotation.y = sinf(pitch);
	rotation.z = cosf(pitch) * sinf(yaw);

	target = DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&rotation));
	UpdateViewDirections();
	UpdateViewMatrix();
}


void Camera::UpdateViewDirections()
{
	//normalize(camera.target - camera.pos)
	forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
	//normalize(cross(forward, up))
	right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, up));
}


void Camera::UpdateViewMatrix()
{
	viewMatrix = DirectX::XMMatrixLookAtLH(position, target, up);

}