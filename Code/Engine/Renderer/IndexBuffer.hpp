#pragma once
#include "Engine/Renderer/Renderer.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ID3D11Buffer;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class IndexBuffer
{
	friend class Renderer;

public:
	IndexBuffer(size_t size, unsigned int stride);
	IndexBuffer(const IndexBuffer& copy) = delete;
	virtual ~IndexBuffer();

	unsigned int const GetStride();

	ID3D11Buffer*	m_buffer = nullptr;
	size_t			m_size = 0;
	unsigned int	m_stride = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------