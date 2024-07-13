#include <d3d11.h>
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct DeferredRenderCommand //A structure that has stuff which is needed for a deferred rendering pipeline
{
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	unsigned int							vertexCount;
	unsigned int							indexCount;
	unsigned int							vertexStride;
	D3D11_PRIMITIVE_TOPOLOGY				topology;
	std::vector<ID3D11ShaderResourceView*>  textures;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------