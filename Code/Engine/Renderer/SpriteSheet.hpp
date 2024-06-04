#pragma once
#include "Engine/Renderer/Texture.hpp"
#include <vector>
#include "Engine/Renderer/SpriteDefinition.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct AABB2;
class SpriteDefinition;
struct Vec2;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class SpriteSheet
{
public:
	explicit SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout);
	explicit SpriteSheet(const Texture& texture, IntVec2 const& textureDimensions, IntVec2 const& simpleGridLayout);
	explicit SpriteSheet(const Texture& texture, IntVec2 const& textureDimensions, IntVec2 const& simpleGridLayout, bool textureLoader);

	const Texture&			    GetTexture() const;
	int						GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	void					GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2					GetSpriteUVs(int spriteIndex) const;

protected:
	const Texture&				m_texture;
	std::vector<SpriteDefinition> m_spriteDefs;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------