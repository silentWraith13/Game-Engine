#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Rgba8.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Image::Image()
	: m_imageFilePath("")
	, m_dimensions()
	, m_texelRgba8Data()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Image::Image(const char* imageFilePath)
	: m_imageFilePath(imageFilePath)
	, m_dimensions()
	, m_texelRgba8Data()
{
	int numComponents = 0;
	stbi_set_flip_vertically_on_load(1);
	unsigned char* imageData = stbi_load(m_imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, STBI_rgb_alpha);
	m_texelRgba8Data.resize(m_dimensions.x * m_dimensions.y);
	memcpy(m_texelRgba8Data.data(), imageData, m_texelRgba8Data.size() * sizeof(Rgba8));
	stbi_image_free(imageData);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Image::Image(IntVec2 size, Rgba8 color)
	: m_imageFilePath("")
	, m_dimensions(size)
	, m_texelRgba8Data(size.x * size.y, color)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const std::string& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const void* Image::GetRawData() const
{
	return m_texelRgba8Data.data();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 Image::GetTexelColor(const IntVec2& texelCoords) const
{
	if (texelCoords.x < 0 || texelCoords.x >= m_dimensions.x || texelCoords.y < 0 || texelCoords.y >= m_dimensions.y)
	{
		return Rgba8(0, 0, 0, 0); 
	}

	int index = texelCoords.x + (texelCoords.y * m_dimensions.x);
	return m_texelRgba8Data[index];
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
size_t Image::GetSizeBytes() const
{
	int totalDim = m_dimensions.x * m_dimensions.y;
	return (totalDim) * sizeof(Rgba8);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Image::~Image()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------