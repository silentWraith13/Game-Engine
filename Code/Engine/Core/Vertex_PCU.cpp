#include "Engine/Core/Vertex_PCU.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& tint, Vec2 const& uvTexCoords) :m_position(position), m_color(tint), m_uvTexCoords(uvTexCoords)
{}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU(Vec2 const& pos, Rgba8 const& color2)
	:m_position(pos.x, pos.y, 0.f),
	 m_color(color2),
	m_uvTexCoords(0.f,0.f)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& tint)
	:m_position(position),
	m_color(tint)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU(Vec2 const& position, Rgba8 const& tint, Vec2 UVs)
	:m_position(position), m_color(tint), m_uvTexCoords(UVs)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
