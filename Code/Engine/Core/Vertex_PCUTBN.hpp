#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vertex_PCUTBN
{
public:

	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
	Vec3 m_tangent;
	Vec3 m_binormal;
	Vec3 m_normal;
public:

	~Vertex_PCUTBN() {}
	Vertex_PCUTBN() {}

	explicit Vertex_PCUTBN(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, Vec3 const& tangent, Vec3 const& biNormal, Vec3 const& normal);
	explicit Vertex_PCUTBN(Vertex_PNCU pncu);


};
//--------------------------------------------------------------------------------------------------------------------------------------------------------