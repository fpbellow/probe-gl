#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	Camera(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& up);

	void Move(const DirectX::XMFLOAT3& direction, float moveSpeed, float deltaTime);
	void Rotate(float yaw, float pitch);

	DirectX::XMVECTOR position;
	DirectX::XMVECTOR target;
	DirectX::XMVECTOR up;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projection;
private:

	void UpdateViewMatrix();
	void UpdateViewDirections();

	DirectX::XMVECTOR right;
	DirectX::XMVECTOR forward;
};