#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ID3D11UnorderedAccessView;
struct ID3D11ShaderResourceView;
struct ID3D11Buffer;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class UnorderedAccessBuffer
{
	friend class Renderer;
public:
	UnorderedAccessBuffer(size_t numElements, unsigned int stride, void* data);
	UnorderedAccessBuffer(const UnorderedAccessBuffer& copy) = delete;
	virtual	~UnorderedAccessBuffer();

	ID3D11UnorderedAccessView*	m_UAV = nullptr;
	ID3D11ShaderResourceView*	m_SRV = nullptr;
	ID3D11Buffer*				m_buffer = nullptr;
	void*						m_data = nullptr;
	size_t						m_numElements = 0;
	size_t						m_stride = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------