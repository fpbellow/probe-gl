#include "../Headers/ResourceLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include"../Headers/stb_image.h"
#include <execution>


Model LoadGLTFFromFile(ID3D11Device* device, const std::filesystem::path& filePath)
{
	
	fastgltf::GltfDataBuffer data;
	data.loadFromFile(filePath);


	fastgltf::Parser parser;
	fastgltf::Options opts = fastgltf::Options::LoadExternalBuffers;
	auto parseResult = parser.loadGltf(&data, filePath.parent_path(), opts);
	if (auto error = parseResult.error(); error != fastgltf::Error::None)
		std::cerr << "fastgltf: Failed to load and parse gltf file " << filePath.string() << " with error " << static_cast<int>(parseResult.error()) << ".\n";

	const fastgltf::Asset& asset = parseResult.get();

	size_t totalPrimitiveCount = 0;
	for (const auto& mesh : asset.meshes)
		totalPrimitiveCount += mesh.primitives.size();


	std::vector<MeshBuffers> outputMeshBuffers(totalPrimitiveCount);
	
	for (const auto& mesh : asset.meshes)
	{
		std::transform(std::execution::par,
			mesh.primitives.begin(), mesh.primitives.end(),
			outputMeshBuffers.begin(),
			[&](const fastgltf::Primitive& primitive)->MeshBuffers {
			
				MeshBuffers outputMesh;
				D3D11_BUFFER_DESC bufferInfo = {};
				D3D11_SUBRESOURCE_DATA resourceData = {};


				//vertex positions data (guaranteed in gltf)
				auto* positionIter = primitive.findAttribute("POSITION");
				const auto& positionAccessor = asset.accessors[positionIter->second];
				outputMesh.vertexCount = positionAccessor.count;
				std::vector<VertexPositionNormalTangentTexture> vertices(outputMesh.vertexCount, {});


				fastgltf::iterateAccessorWithIndex<DirectX::XMFLOAT3>(asset, positionAccessor,
					[&](const DirectX::XMFLOAT3& position, std::size_t i)
					{
						vertices[i].position = position;
					});

				//normal, texture, and tangent vertex data
				if (auto* normalIter = primitive.findAttribute("NORMAL"))
				{
					const auto& normalAccessor = asset.accessors[normalIter->second];
					fastgltf::iterateAccessorWithIndex<DirectX::XMFLOAT3>(asset, normalAccessor,
						[&](const DirectX::XMFLOAT3& normal, std::size_t i)
						{
							vertices[i].normal = normal;
						});
				}

				if (auto* textureIter = primitive.findAttribute("TEXCOORD_0"))
				{
					const auto& textureAccessor = asset.accessors[textureIter->second];
					fastgltf::iterateAccessorWithIndex<DirectX::XMFLOAT2>(asset, textureAccessor,
						[&](const DirectX::XMFLOAT2& texture, std::size_t i)
						{
							vertices[i].texture = texture;
						});
				}

				if (auto* tangentIter = primitive.findAttribute("TANGENT"))
				{
					const auto& tangentAccessor = asset.accessors[tangentIter->second];
					fastgltf::iterateAccessorWithIndex<DirectX::XMFLOAT4>(asset, tangentAccessor,
						[&](const DirectX::XMFLOAT4& tangent, std::size_t i)
						{
							vertices[i].tangent = tangent;
						});
				}

				//indices for index buffer
				std::vector<std::uint32_t> indices;
				if (primitive.indicesAccessor.has_value())
				{
					auto& indAccessor = asset.accessors[primitive.indicesAccessor.value()];
					outputMesh.indexCount = static_cast<UINT>(indAccessor.count);
					indices.resize(outputMesh.indexCount);

					std::size_t i = 0;
					fastgltf::iterateAccessor<std::uint32_t>(asset, indAccessor, [&](std::uint32_t index)
						{
							indices[i++] = index;
						});
				}

				//set the material
				if (primitive.materialIndex.has_value())
					outputMesh.material = *primitive.materialIndex;


				bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexPositionNormalTangentTexture) * vertices.size());
				bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
				bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

				resourceData.pSysMem = vertices.data();
				if (FAILED(device->CreateBuffer(&bufferInfo, &resourceData, &outputMesh.vertexBuffer)))
				{
					std::cerr << "D3D11: Failed to create a gltf primitive vertex buffer. \n";
				}
				bufferInfo.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
				bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
				resourceData.pSysMem = indices.data();

				if (FAILED(device->CreateBuffer(&bufferInfo, &resourceData, &outputMesh.indexBuffer)))
				{
					std::cerr << "D3D11: Failed to create a gltf primitive index buffer. \n";
				}
				return outputMesh;
			
			});

	}

	size_t matCount = asset.materials.size();
	std::vector<PBRMaterialResources> outputMatResources(matCount);

	std::transform(std::execution::par,
		asset.materials.begin(), asset.materials.end(),
		outputMatResources.begin(),
		[&](const fastgltf::Material& material)->PBRMaterialResources {
		
			PBRMaterialResources outputResources;

			outputResources.doublesided = material.doubleSided;

			const std::string& parentPathDir = filePath.parent_path().string() + "/";
			//pbr textures
			if (material.pbrData.baseColorTexture)
			{
				size_t texIndex = material.pbrData.baseColorTexture->textureIndex;
				size_t imgIndex = asset.textures[texIndex].imageIndex.value();
				const fastgltf::Image& image = asset.images[imgIndex];
				if (std::holds_alternative<fastgltf::sources::URI>(image.data))
				{
					std::string fileName{ std::get<fastgltf::sources::URI>(image.data).uri.string() };
					outputResources.baseColorSRV = CreateTextureView(device, parentPathDir + fileName, true);
				}
			}

			if (material.pbrData.metallicRoughnessTexture)
			{
				size_t texIndex = material.pbrData.metallicRoughnessTexture->textureIndex;
				size_t imgIndex = asset.textures[texIndex].imageIndex.value();
				const fastgltf::Image& image = asset.images[imgIndex];
				if (std::holds_alternative<fastgltf::sources::URI>(image.data))
				{
					std::string fileName{ std::get<fastgltf::sources::URI>(image.data).uri.string() };
					outputResources.metallicRoughnessSRV = CreateTextureView(device, parentPathDir + fileName);

				}
			}
			if (material.normalTexture)
			{
				size_t texIndex = material.normalTexture->textureIndex;
				size_t imgIndex = asset.textures[texIndex].imageIndex.value();
				const fastgltf::Image& image = asset.images[imgIndex];
				if (std::holds_alternative<fastgltf::sources::URI>(image.data))
				{
					std::string fileName{ std::get<fastgltf::sources::URI>(image.data).uri.string() };
					outputResources.normalSRV = CreateTextureView(device, parentPathDir + fileName);

				}
			}
			return outputResources;

		});

	std::vector<MaterialConstantBuffer> outputMaterialCBs;
	outputMaterialCBs.resize(matCount);

	for (size_t i = 0; i < matCount; ++i)
	{
		auto const& material = asset.materials[i];
		auto const& bcf = material.pbrData.baseColorFactor;
		auto& bufferData = outputMaterialCBs[i]; 

		bufferData.baseColorFactor = DirectX::XMFLOAT4(bcf[0], bcf[1], bcf[2], bcf[3]); 
		bufferData.alphaMode = static_cast<int>(material.alphaMode); 
		bufferData.alphaCutoff = material.alphaCutoff; 
		bufferData.metallicFactor = material.pbrData.metallicFactor; 
		
	}

	return { outputMeshBuffers, outputMatResources, outputMaterialCBs };
}



WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureView(ID3D11Device* device, const std::string& pathToTexture, bool sRGB)
{
	int width, height, channels;
	unsigned char* data = stbi_load(pathToTexture.c_str(), &width, &height, &channels, 4);

	if (!data)
	{
		std::cerr << "CreateTextureView: Failed to load texture file (" << pathToTexture <<
			"). \n";
		return nullptr;
	}

	
	DXGI_FORMAT textureFormat = sRGB ? DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Format = textureFormat;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Width = static_cast<UINT>(width);
	textureDesc.Height = static_cast<UINT>(height);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	//prepare resource data
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = data;
	initialData.SysMemPitch = static_cast<UINT>(width * 4);

	WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
	if (FAILED(device->CreateTexture2D(&textureDesc, &initialData, texture.GetAddressOf())))
	{
		std::cerr << "CreateTextureView: Failed to create texture from file: " << pathToTexture << ". \n";
		return nullptr;
	}

	ID3D11ShaderResourceView* srv = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	if (FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv)))
	{
		std::cerr << "CreateTextureView: Failed to create SRV from texture" << pathToTexture << ". \n";
		return nullptr;
	}

	stbi_image_free(data);

	return srv;
}

WRL::ComPtr<ID3D11ShaderResourceView> CreateCubeView(ID3D11Device* device, const std::string& folderPath)
{
	std::array<unsigned char*, 6> imageData = {};
	int width = 0, height = 0, channels =0 ;
	
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;


	const std::vector<std::string> faceSides =
	{
		"right.png",  // Positive X
		"left.png",   // Negative X
		"top.png",    // Positive Y
		"bottom.png", // Negative Y
		"front.png",  // Positive Z
		"back.png"    // Negative Z
	};

	std::vector<D3D11_SUBRESOURCE_DATA> subresourceData(6);

	for (int i = 0; i < 6; i++)
	{
		std::string filePath = folderPath + faceSides[i];
		imageData[i] = stbi_load(filePath.c_str(), &width, &height, &channels, 4);

		if (!imageData[i])
		{
			std::cerr << "CreateCubeView: Failed to load texture file (" << filePath <<
				"). \n stb error: " << stbi_failure_reason << std::endl;
			for (int j = 0; j <= i; j++)
				stbi_image_free(imageData[j]);
			return nullptr;
		}

		if (i == 0)
		{
			textureDesc.Width = width;
			textureDesc.Height = height;
		}

		subresourceData[i].pSysMem = imageData[i];
		subresourceData[i].SysMemPitch = static_cast<UINT>(width * 4);
	}

	WRL::ComPtr<ID3D11Texture2D> cubeMapTexture = nullptr;
	if (FAILED(device->CreateTexture2D(&textureDesc, subresourceData.data(), cubeMapTexture.GetAddressOf())))
	{
		std::cerr << "CreateCubeView: Failed to create cubemap texture from files: " << folderPath << ". \n";
		for (auto img : imageData)
			stbi_image_free(img);
		return nullptr;
	}
	for (auto img : imageData)
		stbi_image_free(img);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	ID3D11ShaderResourceView* srv = nullptr;
	if (FAILED(device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, &srv)))
	{
		std::cerr << "CreateCubeView: Failed to create SRV from texture" << folderPath << ". \n";
		return nullptr;
	}

	return srv;
}

WRL::ComPtr<ID3D11ShaderResourceView> LoadProbeCubemaps(ID3D11Device* device, const std::string& pathToExr)
{
	Imf::RgbaInputFile file(pathToExr.c_str());
	Imath::Box2i dw = file.dataWindow();
	int texWidth = dw.max.x - dw.min.x + 1;
	int texHeight = dw.max.y - dw.min.y + 1;
	const int faceSize = 64;

	assert(texWidth % (faceSize * 6) == 0);
	assert(texHeight % faceSize == 0);

	int probesPerRow = texWidth / (faceSize * 6);
	int numRows = texHeight / faceSize;
	int numProbes = probesPerRow * numRows;

	Imf::Array2D<Imf::Rgba> allPixels(texHeight, texWidth);
	file.setFrameBuffer(&allPixels[0][0] - dw.min.x - dw.min.y * texWidth, 1, texWidth);
	file.readPixels(dw.min.y, dw.max.y);

	std::vector<float> faces(numProbes * 6 * faceSize * faceSize * 4);
	for (int row = 0; row < numRows; ++row)
	{
		for (int col = 0; col < probesPerRow; ++col)
		{
			int probeIdx = row * probesPerRow + col;
			for (int face = 0; face < 6; ++face)
			{
				int baseX = (col * 6 + face) * faceSize;
				int baseY = row * faceSize;

				int baseFlat = (probeIdx * 6 + face) * (faceSize * faceSize * 4);

				for (int y = 0; y < faceSize; ++y)
				{
					int py = baseY + y;
					for (int x = 0; x < faceSize; ++x)
					{
						int px = baseX + x;
						int dst_idx = baseFlat + (y * faceSize + x) * 4;
						const Imf::Rgba& src = allPixels[py][px];
						faces[dst_idx + 0] = static_cast<float>(src.r);
						faces[dst_idx + 1] = static_cast<float>(src.g);
						faces[dst_idx + 2] = static_cast<float>(src.b);
						faces[dst_idx + 3] = static_cast<float>(src.a);

					}
				}
			}
		}
	}

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = faceSize;
	texDesc.Height = faceSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = numProbes * 6;
	texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	std::vector<D3D11_SUBRESOURCE_DATA> initData(texDesc.ArraySize);
	for (int i = 0; i < numProbes * 6; ++i)
	{
		initData[i].pSysMem = &faces[i * faceSize * faceSize * 4];
		initData[i].SysMemPitch = faceSize * sizeof(float) * 4;
		initData[i].SysMemSlicePitch = 0;
	}
	

	WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
	if (FAILED(device->CreateTexture2D(&texDesc, initData.data(), texture.GetAddressOf())))
	{
		std::cerr << "LoadProbeData: Failed to create texture from file: " << pathToExr << ". \n";
		return nullptr;
	}

	ID3D11ShaderResourceView* srv = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.Format = texDesc.Format;
	srvDesc.TextureCubeArray.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCubeArray.MostDetailedMip = 0;
	srvDesc.TextureCubeArray.First2DArrayFace = 0;
	srvDesc.TextureCubeArray.NumCubes = numProbes;

	if (FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv)))
	{
		std::cerr << "LoadProbeData: Failed to create SRV from texture" << pathToExr << ". \n";
		return nullptr;
	}

	return srv;
}

IrrProbesConstantBuffer LoadIrrProbesData(const std::string& pathToJson)
{
	IrrProbesConstantBuffer output = {};

	simdjson::padded_string json = simdjson::padded_string::load(pathToJson);
	simdjson::ondemand::parser parser;
	auto doc = parser.iterate(json);


	auto transform = doc["transform"];

	//volume position
	{
		simdjson::ondemand::array posArray;
		auto error = transform["position"].get_array().get(posArray);

		if (error)
		{
			std::cerr << "ParseIrrProbesData: Failed to retrieve volume position. \n";
			return {};
		}

		std::array<double, 3> tempPos;
		size_t i = 0;

		for (auto position : posArray) {
			if (i >= tempPos.size()) 
				break;
		
			tempPos[i] = position;
			++i;
		}
		
		output.volumePosition.x = static_cast<float>(tempPos[0]);
		output.volumePosition.y = static_cast<float>(tempPos[1]);
		output.volumePosition.z = static_cast<float>(tempPos[2]);
	}

	//volume scale
	{
		simdjson::ondemand::array scaleArray;
		auto error = transform["scale"].get_array().get(scaleArray);

		if (error)
		{
			std::cerr << "ParseIrrProbesData: Failed to retrieve volume scales. \n";
			return {};
		}

		std::array<double, 3> tempScale;
		size_t i = 0;

		for (auto scale : scaleArray) {
			if (i >= tempScale.size()) 
				break;
			
			tempScale[i] = scale;
			++i;
		}

		output.volumeScale.x = static_cast<float>(tempScale[0]);
		output.volumeScale.y = static_cast<float>(tempScale[1]);
		output.volumeScale.z = static_cast<float>(tempScale[2]);
	}
	

	auto data = doc["data"];
	//volume resolution
	{
		simdjson::ondemand::array resArray;
		auto error = data["resolution"].get_array().get(resArray);

		if (error)
		{
			std::cerr << "ParseIrrProbesData: Failed to retrieve volume resolution. \n";
			return {};
		}

		std::array<int64_t, 3> tempRes;
		size_t i = 0;

		for (auto res : resArray)
		{
			if (i >= tempRes.size())
				break;

			tempRes[i] = res;
			++i;
		}
		
		output.volumeResolution.x = static_cast<int>(tempRes[0]);
		output.volumeResolution.y = static_cast<int>(tempRes[1]);
		output.volumeResolution.z = static_cast<int>(tempRes[2]);
	}
	
	
	//remaining float params
	{
		double intensity, influenceDist, falloff;
		if (auto err = data["intensity"].get_double().get(intensity))
		{
			std::cerr << "ParseIrrProbesData: Failed to retrieve probes intensity. \n";
			return {};
		}

		if (auto err = data["influence_distance"].get_double().get(influenceDist))
		{
			std::cerr << "ParseIrrProbesData: Failed to retrieve probes influence distance. \n";
			return {};
		}

		if (auto err = data["falloff"].get_double().get(falloff))
		{
			std::cerr << "ParseIrrProbesData: Failed to retrieve probes falloff. \n";
			return {};
		}

		output.intensity = static_cast<float>(intensity);
		output.influenceDistance = static_cast<float>(influenceDist);
		output.falloff = static_cast<float>(falloff);
	}

	return output;
}