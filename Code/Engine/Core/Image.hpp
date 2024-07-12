#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Renderer;
struct Rgba8;
struct IntVec2;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Image
{
	friend class Renderer;

public:
	Image();
	Image(const char* imageFilePath);
	Image(IntVec2 size, Rgba8 color);
	~Image();

	IntVec2				GetDimensions() const;
	const std::string&	GetImageFilePath() const;
	const void*			GetRawData() const;
	Rgba8				GetTexelColor(const IntVec2& texelCoords) const;
	size_t				GetSizeBytes() const;

private:
	std::string			m_imageFilePath;
	IntVec2				m_dimensions;
	std::vector<Rgba8>  m_texelRgba8Data;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------