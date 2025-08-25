#include "../Headers/D3D11Application.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <execution>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")


using VertexType = VertexPositionNormalTangentTexture;

D3D11Application::D3D11Application(const std::string& title) : Application(title)
{
}

D3D11Application::~D3D11Application()
{
	_deviceContext->Flush();
	_rasterStateNone.Reset();
	_renderTarget.Reset();
	_depthTarget.Reset();
	_depthState.Reset();
	_shadowMap.Reset();
	_shadowDSV.Reset();
	_shadowSRV.Reset();
	_shadowRasterState.Reset();
	_comparisonSampleState.Reset();
	_linearSamplerState.Reset();
	_shaderCollection.Destroy();
	_skyShaderCollection.Destroy();
	_shadowShaderCollection.Destroy();
	DestroySwapchainResources();
	_swapChain.Reset();
	_dxgiFactory.Reset();
	_deviceContext.Reset();
	_device.Reset();
}

bool D3D11Application::Initialize()
{
	if (!Application::Initialize())
	{
		return false;
	}


	glfwSetMouseButtonCallback(_window, InputHandler::mouseButtonCallback);
	glfwSetCursorPosCallback(_window, InputHandler::mouseCursorCallback);
	glfwSetKeyCallback(_window, InputHandler::keyCallback);


	//initialize DX device and swapchain
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
	{
		std::cerr << "DXGI: Failed to create DXGI Factory. \n";
		return false;
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
	uint32_t deviceFlags = 0;

	WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		&deviceFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&_device,
		nullptr,
		&deviceContext)))
	{
		std::cerr << "D3D11: Failed to create device and device context. \n";
		return false;
	}

	_deviceContext = deviceContext;



	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
	swapChainDescriptor.Width = GetWindowWidth();
	swapChainDescriptor.Height = GetWindowHeight();
	swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDescriptor.SampleDesc.Count = 1;
	swapChainDescriptor.SampleDesc.Quality = 0;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.BufferCount = 2;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	swapChainDescriptor.Flags = {};

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
	swapChainFullscreenDescriptor.Windowed = true;

	if (FAILED(_dxgiFactory->CreateSwapChainForHwnd(
		_device.Get(),
		glfwGetWin32Window(_window),
		&swapChainDescriptor,
		&swapChainFullscreenDescriptor,
		nullptr,
		&_swapChain
	)))
	{
		std::cerr << "DXGI: Failed to create swapchain. \n";
		return false;
	}

	
	if (!CreateSwapchainResources())
	{
		return false;
	}
	CreateRasterState();
	CreateDepthStencilView();
	CreateDepthState();
	CreateConstantBuffers();
	CreateShadowMapResources();

	if (!ImGuiMenu::Initialize(_window, _device.Get(), _deviceContext.Get()))
		std::cout << "ImGui: Failed to initialize ImGui. \n";

	return true;
}

bool D3D11Application::CreateSwapchainResources()
{
	WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	if (FAILED(_swapChain->GetBuffer( 0, IID_PPV_ARGS(&backBuffer) )))
	{
		std::cerr << "D3D11: Failed to get back buffer from the swap chain. \n";
		return false;
	}


	if (FAILED(_device->CreateRenderTargetView( backBuffer.Get(), nullptr, &_renderTarget )))
	{
		std::cerr << "D3D111: Failed to create render target view from back buffer.\n";
		return false;
	}

	return true;
}

void D3D11Application::DestroySwapchainResources()
{
	_renderTarget.Reset();
}

void D3D11Application::CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.DepthClipEnable = true;

	_device->CreateRasterizerState(&rasterDesc, &_rasterStateNone);

	rasterDesc.CullMode = D3D11_CULL_BACK;
	_device->CreateRasterizerState(&rasterDesc, &_rasterStateBack);


	rasterDesc.DepthBias = 150;            
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 16.0f;
	_device->CreateRasterizerState(&rasterDesc, &_shadowRasterState);
}

void D3D11Application::CreateDepthStencilView()
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Height = GetWindowHeight();
	texDesc.Width = GetWindowWidth();
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.MipLevels = 1;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	ID3D11Texture2D* texture = nullptr;
	if (FAILED(_device->CreateTexture2D(&texDesc, nullptr, &texture)))
	{
		std::cerr << "D3D11: Failed to create texture for DepthStencilView. \n";
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if (FAILED(_device->CreateDepthStencilView(texture, &dsvDesc, &_depthTarget)))
	{
		std::cerr << "D3D11: Failed to create DepthStencilView. \n";
		texture->Release();
		return;
	}

	texture->Release();
}

void D3D11Application::CreateDepthState()
{
	D3D11_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	_device->CreateDepthStencilState(&depthDesc, &_depthState);

	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // no depth writing
	_device->CreateDepthStencilState(&depthDesc, &_depthLessState);

}

void D3D11Application::CreateConstantBuffers()
{
	_perFrameConstantBuffer.Create(_device.Get());
	_lightConstantBuffer.Create(_device.Get());
	_sceneConstantBuffer.Create(_device.Get());
	_probesConstantBuffer.Create(_device.Get());
	
}

void D3D11Application::CreateShadowMapResources()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = _shadowMapDimension;
	textureDesc.Height = _shadowMapDimension;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(_device->CreateTexture2D(&textureDesc, nullptr, &_shadowMap)))
	{
		std::cerr << "D3D11: Failed to create shadow map texture. \n";
	}


	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDepthDesc = {};
	shadowMapDepthDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowMapDepthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowMapDepthDesc.Texture2D.MipSlice = 0;

	if (FAILED(_device->CreateDepthStencilView(_shadowMap.Get(), &shadowMapDepthDesc, &_shadowDSV)))
	{
		std::cerr << "D3D11: Failed to create shadow depth stencil view. \n";
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapResourceViewDesc = {};
	shadowMapResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowMapResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowMapResourceViewDesc.Texture2D.MipLevels = 1;

	if (FAILED(_device->CreateShaderResourceView(_shadowMap.Get(), &shadowMapResourceViewDesc, &_shadowSRV)))
	{
		std::cerr << "D3D11: Failed to create shadow shader resource view. \n";
	}


	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.0f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.0f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	if (FAILED(_device->CreateSamplerState(&comparisonSamplerDesc, &_comparisonSampleState)))
	{
		std::cerr << "D3D11: Failed to create comparison sampler state. \n";
	}

}



bool D3D11Application::Load()
{
	using namespace DirectX;


	ShaderCollectionDescriptor shaderDescriptor = {};
	D3D11_BUFFER_DESC bufferInfo = {};
	D3D11_SUBRESOURCE_DATA resourceData = {};

	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/sky.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/sky.ps.hlsl";
	shaderDescriptor.GeometryShaderFilePath.clear();
	shaderDescriptor.InputElems = VertexPosition::InputElements;
	shaderDescriptor.NumElems = VertexPosition::InputElementCount;

	_skyShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	std::vector<VertexPosition> cVertices = {
		// Front
		{XMFLOAT3(-0.5f,  -0.5f,  0.5f)},  // 0
		{XMFLOAT3(0.5f,  -0.5f,  0.5f)},  // 1
		{XMFLOAT3(-0.5f,   0.5f,  0.5f)},  // 2
		{XMFLOAT3(0.5f,   0.5f,  0.5f)},  // 3

		// Back
		{XMFLOAT3(-0.5f,  -0.5f, -0.5f) }, // 4
		{XMFLOAT3(0.5f,  -0.5f, -0.5f) }, // 5
		{XMFLOAT3(-0.5f,   0.5f, -0.5f)},  // 6
		{XMFLOAT3(0.5f,   0.5f, -0.5f)},  // 7
	};

	constexpr uint32_t indices[] =
	{
		//Front
		3, 2, 0,
		0, 1, 3,

		//Back
		4, 6, 7,
		7, 5, 4,

		//Left
		0, 2, 6,
		6, 4, 0,

		//Right
		7, 3, 1,
		1, 5, 7,

		//Top
		7, 6, 2,
		2, 3, 7,

		//Bottom
		0, 4, 5,
		5, 1, 0

	};

	bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexPosition) * cVertices.size());
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	resourceData.pSysMem = cVertices.data();

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_skyVertexBuffer)))
	{
		std::cerr << "D3D11: Failed to create sky vertex buffer. \n";
		return false;
	}

	bufferInfo.ByteWidth = sizeof(indices);
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

	resourceData.pSysMem = indices;

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_skyIndexBuffer)))
	{
		std::cerr << "D3D11: Failed to create skymap index buffer. \n";
		return false;
	}

	_skyMapSRV = CreateCubeView(_device.Get(), "../Assets/Textures/Skybox/");

	//main scene
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/main.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/main.ps.hlsl";
	shaderDescriptor.GeometryShaderFilePath = L"../Assets/Shaders/main.gs.hlsl";
	shaderDescriptor.InputElems = VertexType::InputElements;
	shaderDescriptor.NumElems = VertexType::InputElementCount;

	_shaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());


	//load sponza model sand store materials
	std::filesystem::path filePath{ "../Assets/Models/Sponza/Sponza.gltf" };
	_menuData.objFile = filePath.filename().string();
	_sponza = LoadGLTFFromFile(_device.Get(), filePath);


	_materialBuffers.resize(_sponza.materials.size());
	std::transform(std::execution::par,
		_sponza.materials.begin(), _sponza.materials.end(),
		_materialBuffers.begin(),
		[&](const MaterialConstantBuffer& bufferData)->ConstantBuffer<MaterialConstantBuffer> 
		{
			ConstantBuffer<MaterialConstantBuffer> materialCB(false);
			materialCB.Create(_device.Get());
			materialCB.Update(_deviceContext.Get(), bufferData);
			return materialCB;
		});

	//load baked irradiance probes 
	_probesIrrCubemaps = LoadProbeCubemaps(_device.Get(), "../Assets/Textures/IrradianceVolume_packed.exr");
	_probesConstantBufferData = LoadIrrProbesData("../Assets/Textures/packed_probe.json");




	D3D11_SAMPLER_DESC linearSamplerStateDescriptor = {};
	linearSamplerStateDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
	linearSamplerStateDescriptor.MaxAnisotropy = 4;
	linearSamplerStateDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamplerStateDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamplerStateDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;

	if (FAILED(_device->CreateSamplerState(&linearSamplerStateDescriptor, &_linearSamplerState)))
	{
		std::cerr << "D3D11: Failed to create linear sampler state. \n";
		return false;
	}

	//shadow shader
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/shadow.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath.clear();
	shaderDescriptor.GeometryShaderFilePath.clear();
	shaderDescriptor.InputElems = VertexType::InputElements;
	shaderDescriptor.NumElems = VertexType::InputElementCount;

	_shadowShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	_mainCamera.projection = XMMatrixPerspectiveFovRH(XM_PIDIV2, static_cast<float>(_width) / static_cast<float>(_height), 0.1f, 1000.0f);

	//model matrix
	static float _scale = 0.1f;
	XMMATRIX scaling = XMMatrixScaling(_scale, _scale, _scale);
	XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMMATRIX modelMatrix = XMMatrixIdentity() * scaling * translation;
	XMMATRIX invTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, modelMatrix));

	XMStoreFloat4x4(&_sceneConstantBufferData.modelMatrix, modelMatrix);
	XMStoreFloat4x4(&_sceneConstantBufferData.invTranspose, invTranspose);

	_sceneConstantBuffer.Update(_deviceContext.Get(), _sceneConstantBufferData);

	_probesConstantBufferData.volumeScale.x *= _scale;
	_probesConstantBufferData.volumeScale.y *= _scale;
	_probesConstantBufferData.volumeScale.z *= _scale;

	_probesConstantBuffer.Update(_deviceContext.Get(), _probesConstantBufferData);

	return true;
}

void D3D11Application::CleanUp()
{
	Application::CleanUp();

}

void D3D11Application::Update()
{
	Application::Update();


	using namespace DirectX;

	_mainCamera.Rotate(InputHandler::camYaw, InputHandler::camPitch);
	_mainCamera.Move(InputHandler::moveDirection, 30.0f, _deltaTime);

	_menuData.camPos = _mainCamera.position;

	XMStoreFloat4x4(&_perFrameConstantBufferData.viewMatrix, _mainCamera.viewMatrix);
	XMStoreFloat4x4(&_perFrameConstantBufferData.projectionMatrix, _mainCamera.projection);
	XMStoreFloat4(&_perFrameConstantBufferData.viewPos, _mainCamera.position);


	XMFLOAT3 lightPosition = XMFLOAT3(52.49f, 84.79f, 7.48f);
	
	XMMATRIX lightView = XMMatrixLookAtRH(XMVectorScale(XMLoadFloat3(&lightPosition),2.4f), XMVectorZero(), {0, 1, 0, 0});
	//XMMATRIX lightProj = XMMatrixOrthographicOffCenterRH(-140.0f, 175.0f, -220.0f, 225.0f, 0.01f, 340.0f);
	XMMATRIX lightProj = XMMatrixOrthographicRH(300.0f, 420.0f, 0.01f, 320.0f );

	_lightConstantBufferData.viewPos = XMFLOAT4(lightPosition.x, lightPosition.y, lightPosition.z, 1.0);
	XMStoreFloat4x4(&_lightConstantBufferData.viewMatrix, lightView);
	XMStoreFloat4x4(&_lightConstantBufferData.projectionMatrix, lightProj);


	_menuData.showMenu = InputHandler::toggleGuiMenu;
	ImGuiMenu::Update(_menuData);

	//update constant buffers
	_perFrameConstantBuffer.Update(_deviceContext.Get(), _perFrameConstantBufferData);

	_lightConstantBuffer.Update(_deviceContext.Get(), _lightConstantBufferData);
}


void D3D11Application::Render()
{
	D3D11_VIEWPORT viewport = {
		0.0f, //TopLeftX
		0.0f, //TopLeftY
		static_cast<float>(GetWindowWidth()),
		static_cast<float>(GetWindowHeight()),
		0.0f, //MinDepth
		1.0f //MaxDepth
	};

	D3D11_VIEWPORT shadowViewport = {
		0.0f, //TopLeftX
		0.0f, //TopLeftY
		static_cast<float>(_shadowMapDimension),
		static_cast<float>(_shadowMapDimension),
		0.0f, //MinDepth
		1.0f //MaxDepth
	};

	constexpr float clearColor[] = { 0.429f, 0.708f, 0.822f, 1.0f };
	UINT stride = sizeof(VertexType);
	constexpr UINT vertexOffset = 0;
	ID3D11RenderTargetView* nullTarget = nullptr;

	ID3D11Buffer* constantBuffers[4] =
	{
		_perFrameConstantBuffer.Get(),
		_lightConstantBuffer.Get(),
		_sceneConstantBuffer.Get(),
		_probesConstantBuffer.Get()
	};

	ImGui::Render();

	// render shadowmap
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
	_deviceContext->ClearDepthStencilView(_shadowDSV.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_deviceContext->OMSetRenderTargets(0, nullptr, _shadowDSV.Get());

	_deviceContext->RSSetViewports(1, &shadowViewport);
	_deviceContext->RSSetState(_shadowRasterState.Get());

	_deviceContext->OMSetDepthStencilState(_depthState.Get(), 0); //enable depth writing

	_shadowShaderCollection.ApplyToContext(_deviceContext.Get());
	_deviceContext->VSSetConstantBuffers(0, 2, &constantBuffers[1]);

	for (MeshBuffers mesh : _sponza.meshes)
	{
		_deviceContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &vertexOffset);
		_deviceContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		_deviceContext->DrawIndexed(mesh.indexCount, 0, 0);
	}

	// render to main back buffer
	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), _depthTarget.Get());
	_deviceContext->ClearDepthStencilView(_depthTarget.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	
	_deviceContext->RSSetViewports(1, &viewport);

	//render skybox
	_deviceContext->OMSetDepthStencilState(_depthLessState.Get(), 0); //disable depth writing
	stride = sizeof(VertexPosition);

	_deviceContext->IASetVertexBuffers(0, 1, _skyVertexBuffer.GetAddressOf(), &stride, &vertexOffset);
	_deviceContext->IASetIndexBuffer(_skyIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	_skyShaderCollection.ApplyToContext(_deviceContext.Get());
	_deviceContext->PSSetShaderResources(0, 1, _skyMapSRV.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffers[0]);

	_deviceContext->DrawIndexed(36, 0, 0);

	//render scene
	_deviceContext->OMSetDepthStencilState(_depthState.Get(), 0); ///enable depth writing
	stride = sizeof(VertexType);
	_shaderCollection.ApplyToContext(_deviceContext.Get());
	_deviceContext->VSSetConstantBuffers(0, 3, constantBuffers);
	_deviceContext->GSSetConstantBuffers(0, 1, &constantBuffers[3]);
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	_deviceContext->PSSetSamplers(1, 1, _comparisonSampleState.GetAddressOf());
	_deviceContext->PSSetShaderResources(3, 1, _shadowSRV.GetAddressOf());
	_deviceContext->PSSetShaderResources(4, 1, _probesIrrCubemaps.GetAddressOf());

	for (MeshBuffers mesh : _sponza.meshes)
	{

	
		const PBRMaterialResources& texture = _sponza.textures[mesh.material];
		ID3D11RasterizerState* activeState =  _rasterStateNone.Get();
		_deviceContext->RSSetState(activeState);

		_deviceContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &vertexOffset);
		_deviceContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		ID3D11Buffer* materialBuffer = _materialBuffers[mesh.material].Get();
		_deviceContext->PSSetConstantBuffers(0, 1, &materialBuffer);

		_deviceContext->PSSetShaderResources(0, 1, texture.baseColorSRV.GetAddressOf());
		_deviceContext->PSSetShaderResources(1, 1, texture.normalSRV.GetAddressOf());
		_deviceContext->PSSetShaderResources(2, 1, texture.metallicRoughnessSRV.GetAddressOf());
		
		_deviceContext->DrawIndexed(mesh.indexCount, 0, 0);
	}
	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	_swapChain->Present(1, 0);
}