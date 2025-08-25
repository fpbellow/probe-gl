#include "../Headers/D3DBufferDefs.hpp"

template<typename T>
ConstantBuffer<T>::ConstantBuffer(const bool dynamic)
{
	m_dynamic = dynamic;
}


template<typename T>
bool ConstantBuffer<T>::Create(ID3D11Device* device)
{
	assert(device);
	D3D11_BUFFER_DESC desc{};
	desc.Usage =m_dynamic ? D3D11_USAGE::D3D11_USAGE_DYNAMIC : D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(T);
	desc.CPUAccessFlags = m_dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	return SUCCEEDED(device->CreateBuffer(&desc, nullptr, m_buffer.ReleaseAndGetAddressOf()));
}

template<typename T>
void ConstantBuffer<T>::Update(ID3D11DeviceContext* deviceContext, const T& data)
{
	assert(deviceContext && m_buffer);
	if (m_dynamic)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		deviceContext->Map(m_buffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &data, sizeof(PerFrameConstantBuffer));
		deviceContext->Unmap(m_buffer.Get(), 0);
	}
	else
	{
		deviceContext->UpdateSubresource(m_buffer.Get(), 0, nullptr, &data, 0, 0);
	}
}


template class ConstantBuffer<PerFrameConstantBuffer>;
template class ConstantBuffer<PerObjectConstantBuffer>;
template class ConstantBuffer<MaterialConstantBuffer>;
template class ConstantBuffer<IrrProbesConstantBuffer>;