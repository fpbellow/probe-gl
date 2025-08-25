#pragma once
#include <DirectXMath.h>
#include <d3d11_2.h>

#include "Definitions.hpp"

struct PerFrameConstantBuffer
{
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4 viewPos;
};

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 modelMatrix;
	DirectX::XMFLOAT4X4 invTranspose;

};

struct MaterialConstantBuffer
{
	DirectX::XMFLOAT4 baseColorFactor = {};
	float metallicFactor = 0.0;
	float alphaCutoff = 0.5f;
	int alphaMode = 0;
	float padding = 0.0f;
};

struct IrrProbesConstantBuffer
{
	DirectX::XMFLOAT3 volumePosition;
	DirectX::XMFLOAT3 volumeScale;
	DirectX::XMINT3 volumeResolution;
	float intensity;
	float influenceDistance;
	float falloff;
};

template<typename T>
class ConstantBuffer
{
public:
	ConstantBuffer() = default;
	ConstantBuffer(const bool dynamic);
	bool Create(ID3D11Device* device);
	void Update(ID3D11DeviceContext* deviceContext, const T& data);

	ID3D11Buffer* Get() { return m_buffer.Get(); }

private:
	bool m_dynamic = true;
	WRL::ComPtr<ID3D11Buffer> m_buffer = nullptr;
};
