#pragma once
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Renderer;
struct  ID3D11Buffer;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class VertexBuffer
{
	friend class Renderer;

public:
	VertexBuffer(size_t size, unsigned int stride);
	VertexBuffer(const VertexBuffer& copy) = delete;
	virtual ~VertexBuffer();
	
	unsigned int const GetStride();

	ID3D11Buffer*	m_buffer;
	size_t			m_size = 0;
	unsigned int	m_stride = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------