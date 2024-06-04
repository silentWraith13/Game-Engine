#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/vertexUtils.hpp"
#include <sstream>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture)
	:m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16,16))
	,m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension)
{
}

BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture, IntVec2 const& textureDimensions) :
	m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension),
	m_fontGlyphsSpriteSheet(fontTexture, textureDimensions, IntVec2(16, 16))
{

}

const Texture& BitmapFont::GetTexture() const
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8(255,255,255)*/, float cellAspect /*= 1.f*/)
{
	int glyphsToDraw = maxGlyphs;
	AABB2 glyphBounds(textMins, textMins);
	for (int charIndex = 0; charIndex < text.length() && glyphsToDraw > 0; charIndex++)
	{
		unsigned char glyph = text[charIndex];
		SpriteDefinition const& spriteDef = m_fontGlyphsSpriteSheet.GetSpriteDef(glyph);
		float glyphAspect = GetGlyphAspect(glyph);
		Vec2 glyphDimensions(cellHeight * cellAspect * glyphAspect, cellHeight);
		glyphBounds.m_maxs = glyphBounds.m_mins + glyphDimensions;
		AddVertsForAABB2D(vertexArray, glyphBounds, tint, spriteDef.GetUVs());
		glyphBounds.m_mins.x += glyphDimensions.x;
		glyphBounds.m_maxs.x += glyphDimensions.x;
		--glyphsToDraw;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect /*= 1.f*/)
{
	float cellWidth = cellAspect * cellHeight;
	float textWidth = cellWidth * text.length();
	return textWidth;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	UNUSED(glyphUnicode);
	return 1.0f;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

float  BitmapFont::GetTextHeight(float cellWidth, std::string const& text, float cellAspect)
{
	float cellHeight = cellAspect / cellWidth;
	float textHeight = cellHeight * text.length();
	return textHeight;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2	const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextDrawMode mode, int	maxGlyphsToDraw)
{

	float boxWidth = box.m_maxs.x - box.m_mins.x;
	float boxHeight = box.m_maxs.y - box.m_mins.y;

	if (mode == TextDrawMode::SHRINK_TO_FIT)
	{
		float textWidth = GetTextWidth(cellHeight, text, cellAspect);
		if (textWidth > boxWidth)
		{
			float overflowWidth = textWidth - boxWidth;
			float overflowPercent = overflowWidth / textWidth;

			cellHeight -= (cellHeight * overflowPercent);
		}

		float textHeight = GetTextHeight(cellHeight, text, cellAspect);
		if (textHeight > boxHeight)
		{
			float overflowHeight = textHeight - boxHeight;
			float overflowPercent = overflowHeight / textHeight;

			cellHeight -= (cellHeight * overflowPercent);
		}
	}

	std::string textToDraw = text;
	if (maxGlyphsToDraw < text.size())
	{
		int totalSpecialChars = 0;
		for (size_t index = 0; index < maxGlyphsToDraw; index++)
		{
			char c = text[index];
			if (c == '\n')
			{
				totalSpecialChars++;
			}
		}

		maxGlyphsToDraw += totalSpecialChars;
		textToDraw = text.substr(0, maxGlyphsToDraw);
	}

	float textHeight = GetTextHeight(cellHeight, text);

	Strings lineSeparatedText = SplitStringOnDelimiter(textToDraw, '\n');
	//Strings lineSeparatedText = SplitStringOnDelimiter(text, '\n');
	int numLines = (int)lineSeparatedText.size();
	for (int lineIndex = 0; lineIndex < numLines; lineIndex++)
	{
		std::string line = lineSeparatedText[lineIndex];

		float lineWidth = GetTextWidth(cellHeight, line, cellAspect);
		//float lineHeight = cellHeight; // change for multi line text

		Vec2 lineMins;
		lineMins.x = box.m_mins.x + ((boxWidth - lineWidth) * alignment.x);
		lineMins.y = box.m_mins.y + ((boxHeight - textHeight) * alignment.y);
		lineMins.y += textHeight - (cellHeight * (float)lineIndex) - cellHeight;

		AddVertsForText2D(vertexArray, lineMins, cellHeight, line, tint, cellAspect);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8(255, 255, 255)*/, float cellAspect /*= 1.f*/, Vec2 const& alignment /*= Vec2(.5f, .5f)*/, int maxGlyphsToDraw /*= 9999999*/)
{
	(void)maxGlyphsToDraw;
	// Step 1: Create the 2D text as normal.
	AddVertsForText2D(verts, textMins, cellHeight, text, tint, cellAspect);

	// Step 2: Determine the 2D bounds of the vertexes.
	AABB2 textBounds = GetVertexBounds2D(verts);

	// Step 3: Create a matrix that transforms the text to 3D space.
	Vec2 textCenter = textBounds.GetCenter();
	Vec3 translation = Vec3(textCenter.x, textCenter.y, 0.f);
	
	// Rotate the text so that it faces the i-axis.
	Vec3 Ibasis;
	Vec3 Jbasis;
	Vec3 Kbasis;
	Mat44 transform;
	transform.SetIJKT3D(Ibasis, Jbasis, Kbasis, translation);

	// Translate the text to be centered, using the 2D bounds and a supplied alignment to determine the translation.
	Vec3 translatedTextPos = Vec3(translation.x * alignment.x, translation.y * alignment.y, 0.f);

	// Step 4: Transform all vertexes by the matrix.
	TransformVertexArray3D(verts, transform);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------