#pragma once

#include "Application.hpp"
#include "ResourceLoader.hpp"

#include "GeometryDefs.hpp"


#include "ShaderCollection.hpp"
#include "InputHandler.hpp"
#include "Camera.hpp"
#include "ImGuiMenu.hpp"
#include "D3DBufferDefs.hpp"





class D3D11Application final : public Application
{

public:
	D3D11Application(const std::string& title);
	~D3D11Application() override;

protected:
	bool Initialize() override;
	bool Load() override;
	void CleanUp() override;
	void Update() override;
	void Render() override;

private:
	void CreateRasterState();
	void CreateDepthStencilView();
	void CreateDepthState();
	void CreateConstantBuffers();
	void CreateShadowMapResources();
	bool CreateSwapchainResources();
	void DestroySwapchainResources();


	WRL::ComPtr<ID3D11Device> _device = nullptr;
	WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
	WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
	WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
	WRL::ComPtr<ID3D11RasterizerState> _rasterStateBack = nullptr;
	WRL::ComPtr<ID3D11RasterizerState> _rasterStateNone = nullptr;
	WRL::ComPtr<ID3D11DepthStencilView> _depthTarget = nullptr;
	WRL::ComPtr<ID3D11DepthStencilState> _depthState = nullptr;
	WRL::ComPtr<ID3D11DepthStencilState> _depthLessState = nullptr;
	WRL::ComPtr<ID3D11Debug> _debug = nullptr;


	ConstantBuffer<PerFrameConstantBuffer> _perFrameConstantBuffer;
	ConstantBuffer<PerFrameConstantBuffer> _lightConstantBuffer;
	ConstantBuffer<PerObjectConstantBuffer> _sceneConstantBuffer;
	std::vector<ConstantBuffer<MaterialConstantBuffer>> _materialBuffers;
	ConstantBuffer<IrrProbesConstantBuffer> _probesConstantBuffer;

	WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> _fallbackTextureSrv = nullptr;

	WRL::ComPtr<ID3D11Texture2D> _shadowMap = nullptr;
	WRL::ComPtr<ID3D11DepthStencilView> _shadowDSV = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> _shadowSRV = nullptr;
	WRL::ComPtr<ID3D11RasterizerState> _shadowRasterState = nullptr;

	WRL::ComPtr<ID3D11SamplerState> _comparisonSampleState = nullptr;

	WRL::ComPtr<ID3D11Buffer> _skyVertexBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> _skyIndexBuffer = nullptr;

	Model _sponza = {};
	WRL::ComPtr<ID3D11ShaderResourceView> _probesIrrCubemaps = nullptr;

	WRL::ComPtr<ID3D11ShaderResourceView> _skyMapSRV = nullptr;

	PerFrameConstantBuffer _perFrameConstantBufferData{};
	PerFrameConstantBuffer _lightConstantBufferData{};
	PerObjectConstantBuffer _sceneConstantBufferData{};
	IrrProbesConstantBuffer _probesConstantBufferData{};

	Camera _mainCamera;

	ShaderCollection _shaderCollection;
	ShaderCollection _shadowShaderCollection;
	ShaderCollection _skyShaderCollection;

	UINT _shadowMapDimension = 8192;

	ImGuiMenuData _menuData;

	
	
};
