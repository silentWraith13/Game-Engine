#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec2.hpp"


//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout)
	:m_texture(texture)
{
	float uPerSpriteX = 1.f / (float)simpleGridLayout.x;
	float vPerSpriteY = 1.f / (float)simpleGridLayout.y;
	int spriteIndex = 0;

	for (int spriteY = 0; spriteY < simpleGridLayout.y; spriteY++)
	{
		for (int spritex = 0; spritex < simpleGridLayout.x; spritex++)
		{
			float minU = uPerSpriteX * (float)spritex;
			float maxU = minU + uPerSpriteX;
			float maxV = 1.0f - (vPerSpriteY * (float)spriteY);
			float minV = maxV - vPerSpriteY;
			m_spriteDefs.push_back(SpriteDefinition(*this, spriteIndex, Vec2(minU, minV), Vec2(maxU, maxV)));
			++spriteIndex;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet(const Texture& texture, IntVec2 const& textureDimensions, IntVec2 const& simpleGridLayout) : m_texture(texture)
{
	int index = 0;
	for (float yIncrement = 1; yIncrement > 0;)
	{
		for (float xIncrement = 0; xIncrement < 1;)
		{
			float xInc = ((float)1 / simpleGridLayout.x) + (1 / (textureDimensions.x * 100));
			float yInc = ((float)1 / simpleGridLayout.y) + (1 / (textureDimensions.y * 100));
			m_spriteDefs.push_back(SpriteDefinition(*this, index, Vec2(xIncrement, yIncrement), Vec2(xIncrement + xInc, yIncrement - yInc)));
			xIncrement += xInc;
			index++;
		}
		yIncrement -= (float)1 / simpleGridLayout.y;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet(const Texture& texture, IntVec2 const& textureDimensions, IntVec2 const& simpleGridLayout, bool textureLoader) : m_texture(texture)
{
	(void)(textureLoader);
	int index = 0;
	for (float yIncrement = 0; yIncrement < 1;)
	{
		for (float xIncrement = 0; xIncrement < 1;)
		{
			float xInc = ((float)1 / simpleGridLayout.x) + (1 / (textureDimensions.x * 100));
			float yInc = ((float)1 / simpleGridLayout.y) + (1 / (textureDimensions.y * 100));
			m_spriteDefs.push_back(SpriteDefinition(*this, index, Vec2(xIncrement, yIncrement), Vec2(xIncrement + xInc, yIncrement - yInc)));
			xIncrement += xInc;
			index++;
		}
		yIncrement += (float)1 / simpleGridLayout.y;
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int SpriteSheet::GetNumSprites() const
{
	return (int)m_spriteDefs.size();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	AABB2 UVs;
	GetSpriteUVs(UVs.m_mins, UVs.m_maxs, spriteIndex);
	return UVs;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------