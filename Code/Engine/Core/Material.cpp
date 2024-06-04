#include "Engine/Core/Material.hpp"
#include "Game/GameCommon.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Material::Material(const char* filePath)
{
	InitializeDefinitions(filePath);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Material::~Material()
{
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Material::InitializeDefinitions(const char* filePath)
{
	XmlDocument materialXML;
	XmlResult result = materialXML.LoadFile(filePath);

	if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{
		ERROR_AND_DIE("Could not open XML file");
	}

	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required model \"%s\"", filePath));

	XmlElement* element = materialXML.RootElement();
	GUARANTEE_OR_DIE(element, Stringf("Failed to open model def element"));
	
	m_name = ParseXmlAttribute(*element, "name", "INVALID MODEL NAME");
	std::string shaderName = ParseXmlAttribute(*element, "shader", "INVALID material SHADER");
	m_shader = g_theRenderer->CreateShaderOrGetFromFile(shaderName.c_str(), m_vertexType);
	if (!m_shader)
	{
		ERROR_AND_DIE("Could not create shader");
	}

	std::string diffuseTexturePath = ParseXmlAttribute(*element, "diffuseTexture", "INVALID diffuse texture path");
	m_diffuseTexture = g_theRenderer->CreateOrGetTextureFromFile(diffuseTexturePath.c_str());
	if (!m_diffuseTexture)
	{
		ERROR_AND_DIE("Could not create diffuse texture");
	}

	std::string normalTexturePath = ParseXmlAttribute(*element, "normalTexture", "INVALID normal Texture path");
	m_normalTextures = g_theRenderer->CreateOrGetTextureFromFile(normalTexturePath.c_str());
	if (!m_normalTextures)
	{
		ERROR_AND_DIE("Could not create normal Texture");
	}

	std::string specGlossTexture = ParseXmlAttribute(*element, "specGlossEmitTexture", "INVALID specGlossEmit Texture path");
	m_specGlossEmitTexture = g_theRenderer->CreateOrGetTextureFromFile(specGlossTexture.c_str());
	if (!m_specGlossEmitTexture)
	{
		ERROR_AND_DIE("Could not create specGlossEmit Texture");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
