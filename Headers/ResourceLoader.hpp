#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/dxmath_element_traits.hpp>

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <simdjson.h>

#include "D3DBufferDefs.hpp"
#include "GeometryDefs.hpp"


#include <filesystem>

#include <string>

#include <vector>
#include <iostream>



struct Model
{
	std::vector<MeshBuffers> meshes;
	std::vector<PBRMaterialResources> textures;
	std::vector<MaterialConstantBuffer> materials;
};


Model LoadGLTFFromFile(ID3D11Device* device, const std::filesystem::path& filePath);
WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureView(ID3D11Device* device, const std::string& pathToTexture, bool sRGB = false);
WRL::ComPtr<ID3D11ShaderResourceView> LoadProbeCubemaps(ID3D11Device* device, const std::string& pathToExr);
WRL::ComPtr<ID3D11ShaderResourceView> CreateCubeView(ID3D11Device* device, const std::string& folderPath);
IrrProbesConstantBuffer LoadIrrProbesData(const std::string& pathToJson);

