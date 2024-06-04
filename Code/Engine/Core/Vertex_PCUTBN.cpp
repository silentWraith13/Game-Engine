#include "Engine/Core/Vertex_PCUTBN.hpp"


//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vertex_PCUTBN::Vertex_PCUTBN(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, Vec3 const& tangent, Vec3 const& biNormal, Vec3 const& normal)
	:m_position(position), m_color(color), m_uvTexCoords(uvTexCoords), m_tangent(tangent), m_binormal(biNormal), m_normal(normal)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vertex_PCUTBN::Vertex_PCUTBN(Vertex_PNCU pncu)
{
	m_position = pncu.m_position;
	m_normal = pncu.m_normal;
	m_color = pncu.m_color;
	m_uvTexCoords = pncu.m_uvTexCoords;
	m_binormal = Vec3(0.f, 0.f, 0.f);
	m_tangent = Vec3(0.f, 0.f, 0.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------