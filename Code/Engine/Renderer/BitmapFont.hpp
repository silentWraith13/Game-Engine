#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

struct Vec2;
class Texture;
struct Vertex_PCU;
struct Rgba8;

enum TextDrawMode
{
	SHRINK_TO_FIT,
	OVERRUN
};

class BitmapFont
{
	friend class Renderer; // Only the Renderer can create new BitmapFont objects!

public:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture);
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture, IntVec2 const& textureDimensions);

	const Texture& GetTexture() const;

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8(255,255,255), float cellAspect = 1.f);
	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2	const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextDrawMode mode, int	maxGlyphs = 9999999);
	void AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8(255, 255, 255), float cellAspect = 1.f, Vec2 const& alignment = Vec2(.5f, .5f), int maxGlyphsToDraw = 9999999);

	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f);
	float GetTextHeight(float cellWidth, std::string const& text, float cellAspect = 1.f);

protected:
	float GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
	int maxGlyphs = 9999;
};
