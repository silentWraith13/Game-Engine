#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class SpriteSheet;
class Texture;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class SpriteDefinition
{
public:
	explicit SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs);
	
	void					GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const;
	AABB2					GetUVs() const;
	SpriteSheet const&		GetSpriteSheet() const;
	const Texture&			GetTexture() const;
	float					GetAspect() const;

	const SpriteSheet&			m_spriteSheet;
	int							m_spriteIndex = -1;
	Vec2						m_uvAtMins = Vec2(0.f,0.f);
	Vec2						m_uvAtMaxs = Vec2(1.f, 1.f);

};
//--------------------------------------------------------------------------------------------------------------------------------------------------------