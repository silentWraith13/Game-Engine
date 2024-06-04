#pragma once
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/XmlUtils.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Material
{
public:
	Material(const char* filePath);
	~Material();
	void InitializeDefinitions(const char* filePath);

	std::string m_name;
	Shader* m_shader;
	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTextures = nullptr;
	Texture* m_specGlossEmitTexture = nullptr;
	Rgba8 color = Rgba8::WHITE;
	VertexType m_vertexType = VertexType::PCUTBN;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------