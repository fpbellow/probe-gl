#include "../Headers/ShaderCollection.hpp"

ShaderCollection ShaderCollection::CreateShaderCollection(const ShaderCollectionDescriptor& settings, ID3D11Device* device)
{
    ShaderCollection collection;

    WRL::ComPtr<ID3DBlob> vertexShaderBlob;

    if (!settings.VertexShaderFilePath.empty())
        collection._vertexShader = CreateVertexShader(device, settings.VertexShaderFilePath, vertexShaderBlob);

    if (!settings.PixelShaderFilePath.empty())
        collection._pixelShader = CreatePixelShader(device, settings.PixelShaderFilePath);

    if (!settings.GeometryShaderFilePath.empty())
        collection._geometryShader = CreateGeometryShader(device, settings.GeometryShaderFilePath);


    if (!settings.HullShaderFilePath.empty() || !settings.DomainShaderFilePath.empty())
    {
        collection._hullShader = CreateHullShader(device, settings.HullShaderFilePath);
        collection._domainShader = CreateDomainShader(device, settings.DomainShaderFilePath);
    }


    if (collection._vertexShader != nullptr && !CreateInputLayout(device, settings.InputElems, settings.NumElems, vertexShaderBlob, collection._inputLayout))
        return {};

    return collection;
}



WRL::ComPtr<ID3D11VertexShader> ShaderCollection::CreateVertexShader(ID3D11Device* device, const std::wstring& filePath, WRL::ComPtr<ID3DBlob>& vertexShaderBlob)
{
    if (!CompileShader(filePath, "Main", "vs_5_0", vertexShaderBlob))
        return nullptr;

    WRL::ComPtr<ID3D11VertexShader> vertexShader;
    if (FAILED(device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader
    )))
    {
        std::cerr << "D3D11: Failed to compile vertex shader. \n";
        return nullptr;
    }

    return vertexShader;
}


WRL::ComPtr<ID3D11PixelShader> ShaderCollection::CreatePixelShader(ID3D11Device* device, const std::wstring& filePath)
{
    WRL::ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
    if (!CompileShader(filePath, "Main", "ps_5_0", pixelShaderBlob))
        return nullptr;

    WRL::ComPtr<ID3D11PixelShader> pixelShader;
    if (FAILED(device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader
    )))
    {
        std::cerr << "D3D11: Failed to compile pixel shader. \n";
        return nullptr;
    }

    return pixelShader;
}

WRL::ComPtr<ID3D11GeometryShader> ShaderCollection::CreateGeometryShader(ID3D11Device* device, const std::wstring& filePath)
{
    WRL::ComPtr<ID3DBlob> geometryShaderBlob = nullptr;
    if (!CompileShader(filePath, "Main", "gs_5_0", geometryShaderBlob))
        return nullptr;

    WRL::ComPtr<ID3D11GeometryShader> geometryShader;
    if (FAILED(device->CreateGeometryShader(
        geometryShaderBlob->GetBufferPointer(),
        geometryShaderBlob->GetBufferSize(),
        nullptr,
        &geometryShader
    )))
    {
        std::cerr << "D3D11: Failed to compile geometry shader. \n";
        return nullptr;
    }

    return geometryShader;
}

WRL::ComPtr<ID3D11HullShader> ShaderCollection::CreateHullShader(ID3D11Device* device, const std::wstring& filePath)
{
    WRL::ComPtr<ID3DBlob> hullShaderBlob = nullptr;
    if (!CompileShader(filePath, "Main", "hs_5_0", hullShaderBlob))
        return nullptr;

    WRL::ComPtr<ID3D11HullShader> hullShader;
    if (FAILED(device->CreateHullShader(
        hullShaderBlob->GetBufferPointer(),
        hullShaderBlob->GetBufferSize(),
        nullptr,
        &hullShader
    )))
    {
        std::cerr << "D3D11: Failed to compile hull shader. \n";
        return nullptr;
    }

    return hullShader;
}

WRL::ComPtr<ID3D11DomainShader> ShaderCollection::CreateDomainShader(ID3D11Device* device, const std::wstring& filePath)
{
    WRL::ComPtr<ID3DBlob> domainShaderBlob = nullptr;
    if (!CompileShader(filePath, "Main", "ds_5_0", domainShaderBlob))
        return nullptr;

    WRL::ComPtr<ID3D11DomainShader> domainShader;
    if (FAILED(device->CreateDomainShader(
        domainShaderBlob->GetBufferPointer(),
        domainShaderBlob->GetBufferSize(),
        nullptr,
        &domainShader
    )))
    {
        std::cerr << "D3D11: Failed to compile domain shader. \n";
        return nullptr;
    }

    return domainShader;
}



bool ShaderCollection::CreateInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* InputElements, UINT InputElementCount, const WRL::ComPtr<ID3DBlob>& vertexBlob, WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{

    if (FAILED(device->CreateInputLayout(
        InputElements,
        InputElementCount,
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        &inputLayout
    )))
    {
        std::cerr << "D3D11: Failed to create the input layout. \n";
        return false;
    }

    return true;
}


bool ShaderCollection::CompileShader(const std::wstring& filePath, const std::string& entryPoint, const std::string& profile, WRL::ComPtr<ID3DBlob>& shaderBlob)
{
    constexpr uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;


    WRL::ComPtr<ID3DBlob> tempShaderBlob = nullptr;
    WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    if (FAILED(D3DCompileFromFile(
        filePath.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.data(),
        profile.data(),
        compileFlags,
        0,
        &tempShaderBlob,
        &errorBlob
    )))
    {
        std::wcerr << "D3D11: Failed to read shader from file: " << filePath << " \n";
        if (errorBlob != nullptr)
            std::cerr << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";

        return false;
    }

    shaderBlob = tempShaderBlob;
    return true;
}


void ShaderCollection::ApplyToContext(ID3D11DeviceContext* context)
{
    context->IASetInputLayout(_inputLayout.Get());
    context->GSSetShader(nullptr, nullptr, 0);

    context->VSSetShader(_vertexShader.Get(), nullptr, 0);

    if (_pixelShader != nullptr)
        context->PSSetShader(_pixelShader.Get(), nullptr, 0);

    if (_geometryShader != nullptr)
    {
        context->GSSetShader(_geometryShader.Get(), nullptr, 0);
    }

    if (_hullShader != nullptr || _domainShader != nullptr)
    {
        context->HSSetShader(_hullShader.Get(), nullptr, 0);
        context->DSSetShader(_domainShader.Get(), nullptr, 0);
    }


}


void ShaderCollection::Destroy()
{
    _vertexShader.Reset();
    _pixelShader.Reset();
    _geometryShader.Reset();
    _hullShader.Reset();
    _domainShader.Reset();
    _inputLayout.Reset();
}