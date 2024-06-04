#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <wrl/client.h>
#include <d3d12.h>
#pragma once
enum class TextureType
{
	WICT,
	DDS
};
class TextureD12
{
	friend class Renderer; // Only the Renderer can create new Texture objects!
	friend class RendererD12; // Only the Renderer can create new Texture objects!

private:
	TextureD12() = default; // can't instantiate directly; must ask Renderer to do it for you
	TextureD12(TextureD12 const& copy) = delete; // No copying allowed!  This represents GPU memory.
	~TextureD12();

public:
	IntVec2				GetDimensions() const { return m_dimensions; }
	std::string const&  GetImageFilePath() const { return m_path; }
	std::string const&  GetImageFileName() const { return m_name; }

public:
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_textureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_uploadResource;      // TODO: release after initialization
	D3D12_CPU_DESCRIPTOR_HANDLE						m_cpuDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_gpuDescriptorHandle;
	UINT											m_heapIndex = UINT_MAX;
	D3D12_SRV_DIMENSION								m_textureSRVDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	TextureType										m_textureType;
protected:
	std::string										m_name;
	std::string										m_path;
	IntVec2											m_dimensions;


	// #ToDo: generalize/replace this for D3D11 support!
	//unsigned int				m_openglTextureID = 0xFFFFFFFF;
	/*unsigned int				m_textureID = 0xFFFFFFFF;*/

};

