#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vertex_PNCU
{
public:

	Vec3 m_position;
	Vec3 m_normal;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

public:

	~Vertex_PNCU() {}
	Vertex_PNCU() {}

	explicit Vertex_PNCU(Vec3 const& position, Vec3 normal , Rgba8 const& tint, Vec2 const& uvTexCoords);
	explicit Vertex_PNCU(Vec3 const& position, Vec3 normal, Rgba8 const& tint);
	explicit Vertex_PNCU(Vec2 const& position, Rgba8 const& tint);


};
//--------------------------------------------------------------------------------------------------------------------------------------------------------