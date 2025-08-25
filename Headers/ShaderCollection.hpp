#pragma once

#include "Definitions.hpp"

#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <iostream>


struct ShaderCollectionDescriptor
{
	std::wstring VertexShaderFilePath;
	std::wstring PixelShaderFilePath;
	std::wstring GeometryShaderFilePath;
	std::wstring HullShaderFilePath;
	std::wstring DomainShaderFilePath;
	const D3D11_INPUT_ELEMENT_DESC* InputElems;
	UINT NumElems;
};

class ShaderCollection
{

public:

	static ShaderCollection CreateShaderCollection(const ShaderCollectionDescriptor& settings, ID3D11Device* device);

	void ApplyToContext(ID3D11DeviceContext* context);
	void Destroy();

private:

	static [[nodiscard]] WRL::ComPtr <ID3D11VertexShader> CreateVertexShader
	(
		ID3D11Device* device,
		const std::wstring& filePath,
		WRL::ComPtr<ID3DBlob>& vertexShaderBlob
	);

	static [[nodiscard]] WRL::ComPtr<ID3D11PixelShader> CreatePixelShader
	(
		ID3D11Device* device,
		const std::wstring& filePath
	);

	static [[nodiscard]] WRL::ComPtr <ID3D11GeometryShader> CreateGeometryShader
	(
		ID3D11Device* device,
		const std::wstring& filePath
	);

	static [[nodiscard]] WRL::ComPtr <ID3D11HullShader> CreateHullShader
	(
		ID3D11Device* device,
		const std::wstring& filePath
	);

	static [[nodiscard]] WRL::ComPtr <ID3D11DomainShader> CreateDomainShader
	(
		ID3D11Device* device,
		const std::wstring& filePath
	);

	static bool CreateInputLayout
	(
		ID3D11Device* device,
		const D3D11_INPUT_ELEMENT_DESC* InputElements,
		UINT InputElementCount,
		const WRL::ComPtr<ID3DBlob>& vertexBlob,
		WRL::ComPtr<ID3D11InputLayout>& inputLayout
	);

	static bool CompileShader
	(
		const std::wstring& filePath,
		const std::string& entryPoint,
		const std::string& profile,
		WRL::ComPtr<ID3DBlob>& shaderBlob
	);


	WRL::ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
	WRL::ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
	WRL::ComPtr<ID3D11GeometryShader> _geometryShader = nullptr;
	WRL::ComPtr<ID3D11HullShader> _hullShader = nullptr;
	WRL::ComPtr<ID3D11DomainShader> _domainShader = nullptr;
	WRL::ComPtr<ID3D11InputLayout> _inputLayout = nullptr;
	uint32_t _vertexSize = 0;

};