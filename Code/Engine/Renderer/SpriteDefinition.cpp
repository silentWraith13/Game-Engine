#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteDefinition::SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
	:m_spriteSheet(spriteSheet), m_spriteIndex(spriteIndex),m_uvAtMins(uvAtMins), m_uvAtMaxs(uvAtMaxs)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void SpriteDefinition::GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 SpriteDefinition::GetUVs() const
{
	return AABB2(m_uvAtMins,m_uvAtMaxs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteSheet const& SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
const Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float SpriteDefinition::GetAspect() const
{
	Vec2 dimension = GetUVs().GetDimensions();
	return dimension.x / dimension.y;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
