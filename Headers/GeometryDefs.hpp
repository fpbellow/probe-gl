#pragma once
#include "Definitions.hpp"
#include <d3d11_2.h>
#include <DirectXMath.h>

struct VertexPositionNormalTangentTexture
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT2 texture;

	static constexpr unsigned int InputElementCount = 4;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct VertexPosition
{
	DirectX::XMFLOAT3 position;
	static constexpr unsigned int InputElementCount = 1;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};


struct MeshBuffers {
	WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> indexBuffer = nullptr;
	size_t vertexCount = 0;
	UINT indexCount = 0;
	size_t material = 0;
};


struct PBRMaterialResources {
	bool doublesided = false;

	WRL::ComPtr<ID3D11ShaderResourceView> baseColorSRV = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> metallicRoughnessSRV = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> normalSRV = nullptr;
};

