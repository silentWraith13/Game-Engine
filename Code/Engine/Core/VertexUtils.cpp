#include "Engine/Core/vertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr float PI = 3.1415926535897932384626433832795f;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)			///#ToDo::Could make this wayyy faster.
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, uniformScaleXY, rotationDegreesAboutZ, translationXY);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, const Mat44& transform)
{
	for (int index = 0; index < numVerts; index++)
	{
		Vec3& pos = verts[index].m_position;
		pos = transform.TransformPosition3D(pos);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (int i = 0; i < (int)verts.size(); ++i)
	{
		Vec3& pos = verts[i].m_position;
		pos = transform.TransformPosition3D(pos);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D(std::vector<Vertex_PNCU>& verts, const Mat44& transform)
{
	for (int i = 0; i < (int)verts.size(); ++i)
	{
		Vec3& pos = verts[i].m_position;
		pos = transform.TransformPosition3D(pos);

		Vec3& normal = verts[i].m_normal;
		normal = transform.TransformVectorQuantity3D(normal);
		normal.Normalize();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform)
{
	for (int i = 0; i < (int)verts.size(); ++i)
	{
		Vec3& pos = verts[i].m_position;
		pos = transform.TransformPosition3D(pos);

		Vec3& normal = verts[i].m_normal;
		normal = transform.TransformVectorQuantity3D(normal);
		normal.Normalize();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void RotateVertexArrayAroundCenterXY3D(int numVerts, Vertex_PCU* verts, float rotationDegreesAboutZ, Vec2 const& centerOfRotation)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;

		// Translate vertices so that the centerOfRotation becomes the origin
		pos.x -= centerOfRotation.x;
		pos.y -= centerOfRotation.y;

		// Rotate around the new origin (which is the centerOfRotation)
		float thetaRadians = atan2f(pos.y, pos.x) + ConvertDegreesToRadians(rotationDegreesAboutZ);
		float length = sqrtf(pos.x * pos.x + pos.y * pos.y);
		pos.x = length * cosf(thetaRadians);
		pos.y = length * sinf(thetaRadians);

		// Translate vertices back to their original position
		pos.x += centerOfRotation.x;
		pos.y += centerOfRotation.y;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 CalculateBounds(const std::vector<Vertex_PCU>& verts)
{
	if (verts.empty())
	{
		return AABB2(); // Return an empty bounds if there are no vertices
	}

	float minX = verts[0].m_position.x;
	float maxX = minX;
	float minY = verts[0].m_position.y;
	float maxY = minY;

	for (const Vertex_PCU& vert : verts)
	{
		if (vert.m_position.x < minX) minX = vert.m_position.x;
		if (vert.m_position.x > maxX) maxX = vert.m_position.x;
		if (vert.m_position.y < minY) minY = vert.m_position.y;
		if (vert.m_position.y > maxY) maxY = vert.m_position.y;
	}

	return AABB2(Vec2(minX, minY), Vec2(maxX, maxY));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void CalculateTangentsAndBiTangents(std::vector<Vertex_PCUTBN>& vertices, const std::vector<unsigned int>& indices)
{
	std::vector<Vec3> tempTangents(vertices.size(), Vec3(0.f, 0.f, 0.f));
	std::vector<Vec3> tempBitangents(vertices.size(), Vec3(0.f, 0.f, 0.f));

	for (int i = 0; i < (int)indices.size(); i += 3) 
	{
		unsigned int i0 = indices[i];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		Vec3 p0 = vertices[i0].m_position;
		Vec3 p1 = vertices[i1].m_position;
		Vec3 p2 = vertices[i2].m_position;

		Vec2 w0 = vertices[i0].m_uvTexCoords;
		Vec2 w1 = vertices[i1].m_uvTexCoords;
		Vec2 w2 = vertices[i2].m_uvTexCoords;

		Vec3 e1 = p1 - p0;
		Vec3 e2 = p2 - p0;
		float x1 = w1.x - w0.x;
		float x2 = w2.x - w0.x;
		float y1 = w1.y - w0.y;
		float y2 = w2.y - w0.y;

		float r = 1.f / (x1 * y2 - x2 * y1);

		Vec3 t = (e1 * y2 - e2 * y1) * r;
		Vec3 b = (e2 * x1 - e1 * x2) * r;
		b.GetNormalized();
		t.GetNormalized();
		
		tempTangents[i0] += t;
		tempTangents[i1] += t;
		tempTangents[i2] += t;
		
		tempBitangents[i0] += b;
		tempBitangents[i1] += b;
		tempBitangents[i2] += b;
	}

	for (int i = 0; i < (int)vertices.size(); ++i)
	{
		Vec3 t = tempTangents[i];
		Vec3 b = tempBitangents[i];
		Vec3 n = vertices[i].m_normal;

		// Orthonormalize T (tangent) against N (normal)
		t = (t - n * DotProduct3D(n, t)).GetNormalized();

		// Orthonormalize B (bitangent) against both T (tangent) and N (normal)
		b = (b - t * DotProduct3D(t, b) - n * DotProduct3D(n, b)).GetNormalized();

		vertices[i].m_tangent = t;
		vertices[i].m_binormal = b;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 BL = Vec3( bounds.m_mins.x, bounds.m_mins.y, 0.f);
	Vec3 BR = Vec3( bounds.m_maxs.x, bounds.m_mins.y, 0.f);
	Vec3 TR = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, 0.f);
	Vec3 TL = Vec3( bounds.m_mins.x, bounds.m_maxs.y, 0.f);

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back( Vertex_PCU( BL, color, uvBL ) );
	verts.push_back( Vertex_PCU( BR, color, uvBR ) );
	verts.push_back( Vertex_PCU( TR, color, uvTR ) );
	
	verts.push_back( Vertex_PCU( BL, color, uvBL ) );
	verts.push_back( Vertex_PCU( TR, color, uvTR ) );
	verts.push_back( Vertex_PCU( TL, color, uvTL ) );
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForHollowAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/, Rgba8 const& innerColor /*= Rgba8::BLACK*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	// Outer bounds vertices
	Vec2 oBL = bounds.m_mins;
	Vec2 oTR = bounds.m_maxs;

	// Calculate the coordinates for the corners of the inner border (inner box)
	Vec2 iBL = bounds.m_mins + Vec2(thickness, thickness);
	Vec2 iTR = bounds.m_maxs - Vec2(thickness, thickness);

	// Calculate the corners of the inner box for easy reference
	Vec2 iBR(iTR.x, iBL.y);
	Vec2 iTL(iBL.x, iTR.y);

	// Top border
	AddVertsForAABB2D(verts, AABB2(iTL, Vec2(iTR.x, oTR.y)), color, UVs);
	// Bottom border
	AddVertsForAABB2D(verts, AABB2(oBL, Vec2(iBR.x, iBL.y)), color, UVs);
	// Left border
	AddVertsForAABB2D(verts, AABB2(oBL, Vec2(iBL.x, oTR.y)), color, UVs);
	// Right border
	AddVertsForAABB2D(verts, AABB2(iBR, oTR), color, UVs);

	// Fill the inner box
	AddVertsForAABB2D(verts, AABB2(iBL, iTR), innerColor, UVs);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color /*= Rgba8(255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f) )*/)
{
	Vec3 min = bounds.m_mins;
	Vec3 max = bounds.m_maxs;

	Vec3 BLF = Vec3(min.x, min.y, min.z);
	Vec3 BRF = Vec3(max.x, min.y, min.z);
	Vec3 TRF = Vec3(max.x, min.y, max.z);
	Vec3 TLF = Vec3(min.x, min.y, max.z);

	Vec3 BLB = Vec3(max.x, max.y, min.z);
	Vec3 BRB = Vec3(min.x, max.y, min.z);
	Vec3 TRB = Vec3(min.x, max.y, max.z);
	Vec3 TLB = Vec3(max.x, max.y, max.z);

	// Front face
	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TLF, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

	//Left face
	Vec2 left_uv_min = UVs.m_mins;
	Vec2 left_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));


	// Back face
	Vec2 back_uv_min = UVs.m_mins;
	Vec2 back_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

	// Right face
	Vec2 right_uv_min = UVs.m_mins;
	Vec2 right_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));


	// Top face
	Vec2 top_uv_min = UVs.m_mins;
	Vec2 top_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(TLF, color, Vec2(top_uv_min.x, top_uv_min.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(top_uv_max.x, top_uv_min.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(top_uv_max.x, top_uv_max.y)));

	verts.push_back(Vertex_PCU(TLF, color, Vec2(top_uv_min.x, top_uv_min.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(top_uv_max.x, top_uv_max.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(top_uv_min.x, top_uv_max.y)));

	// Bottom face
	Vec2 bottom_uv_min = UVs.m_mins;
	Vec2 bottom_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForIndexedQuadPCUTBN(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexedVerts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	unsigned int vertOffset = (unsigned int)verts.size();

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	
	Vec3 normal = CrossProduct3D((bottomRight - bottomleft).GetNormalized(), (topLeft - bottomleft).GetNormalized()).GetNormalized();
 	Vec3 tangent = Vec3(0.f, 0.f, 0.f);
 	Vec3 biNormal = Vec3(0.f, 0.f, 0.f);
	verts.push_back(Vertex_PCUTBN(bottomleft, color, uvBL, tangent, biNormal, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, uvBR,tangent, biNormal, normal));
	verts.push_back(Vertex_PCUTBN(topRight,  color, uvTR,tangent, biNormal, normal));
	verts.push_back(Vertex_PCUTBN(topLeft, color, uvTL,tangent, biNormal, normal));

	indexedVerts.push_back(vertOffset + 0);
	indexedVerts.push_back(vertOffset + 1);
	indexedVerts.push_back(vertOffset + 2);

	indexedVerts.push_back(vertOffset + 0);
	indexedVerts.push_back(vertOffset + 2);
	indexedVerts.push_back(vertOffset + 3);

	CalculateTangentsAndBiTangents(verts, indexedVerts);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForIndexedCube(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	Vec3 min = bounds.m_mins;
	Vec3 max = bounds.m_maxs;

	Vec3 BLF = Vec3(min.x, min.y, min.z);
	Vec3 BRF = Vec3(max.x, min.y, min.z);
	Vec3 TRF = Vec3(max.x, min.y, max.z);
	Vec3 TLF = Vec3(min.x, min.y, max.z);

	Vec3 BLB = Vec3(max.x, max.y, min.z);
	Vec3 BRB = Vec3(min.x, max.y, min.z);
	Vec3 TRB = Vec3(min.x, max.y, max.z);
	Vec3 TLB = Vec3(max.x, max.y, max.z);

	// Front face
	AddVertsForIndexedQuadPCUTBN(verts, indexes, BLF, BRF, TRF, TLF, color, UVs);

	//Left
	AddVertsForIndexedQuadPCUTBN(verts, indexes, BRB, BLF, TLF, TRB, color, UVs);

	// Back face
	AddVertsForIndexedQuadPCUTBN(verts, indexes, BLB, BRB, TRB, TLB, color, UVs);

	// Right face
	AddVertsForIndexedQuadPCUTBN(verts, indexes, BRF, BLB, TLB, TRF, color, UVs);

	// Top face
	AddVertsForIndexedQuadPCUTBN(verts, indexes, TLF, TRF, TLB, TRB, color, UVs);
	
	// Bottom face
	AddVertsForIndexedQuadPCUTBN(verts, indexes, BRB, BLB, BRF, BLF, color, UVs);

	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForAABBZ3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	Vec3 min = bounds.m_mins;
	Vec3 max = bounds.m_maxs;

	Vec3 BLF = Vec3(min.x, min.y, min.z);
	Vec3 BRF = Vec3(max.x, min.y, min.z);
	Vec3 TRF = Vec3(max.x, min.y, max.z);
	Vec3 TLF = Vec3(min.x, min.y, max.z);

	Vec3 BLB = Vec3(max.x, max.y, min.z);
	Vec3 BRB = Vec3(min.x, max.y, min.z);
	Vec3 TRB = Vec3(min.x, max.y, max.z);
	Vec3 TLB = Vec3(max.x, max.y, max.z);

	// Front face
	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TLF, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

	//Left face
	Vec2 left_uv_min = UVs.m_mins;
	Vec2 left_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));


	// Back face
	Vec2 back_uv_min = UVs.m_mins;
	Vec2 back_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

	// Right face
	Vec2 right_uv_min = UVs.m_mins;
	Vec2 right_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));


	// Top face
	Vec2 top_uv_min = UVs.m_mins;
	Vec2 top_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(TLF, color, Vec2(top_uv_min.x, top_uv_min.y)));
	verts.push_back(Vertex_PCU(TRF, color, Vec2(top_uv_max.x, top_uv_min.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(top_uv_max.x, top_uv_max.y)));

	verts.push_back(Vertex_PCU(TLF, color, Vec2(top_uv_min.x, top_uv_min.y)));
	verts.push_back(Vertex_PCU(TLB, color, Vec2(top_uv_max.x, top_uv_max.y)));
	verts.push_back(Vertex_PCU(TRB, color, Vec2(top_uv_min.x, top_uv_max.y)));

	// Bottom face
	Vec2 bottom_uv_min = UVs.m_mins;
	Vec2 bottom_uv_max = UVs.m_maxs;
	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BLB, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(BRB, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(BRF, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(BLF, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForAABBWireFrame3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds,  Rgba8 const& color /*= Rgba8::WHITE*/)
{
	// Calculate the eight corners of the AABB3
	Vec3 min = bounds.m_mins;
	Vec3 max = bounds.m_maxs;


	Vec3 BLF = Vec3(min.x, min.y, min.z);
	Vec3 BRF = Vec3(max.x, min.y, min.z);
	Vec3 TRF = Vec3(max.x, min.y, max.z);
	Vec3 TLF = Vec3(min.x, min.y, max.z);


	Vec3 BLB = Vec3(max.x, max.y, min.z);
	Vec3 BRB = Vec3(min.x, max.y, min.z);
	Vec3 TRB = Vec3(min.x, max.y, max.z);
	Vec3 TLB = Vec3(max.x, max.y, max.z);

	// Front face
	verts.push_back(Vertex_PCU(BLF, color));
	verts.push_back(Vertex_PCU(BRF, color));
	verts.push_back(Vertex_PCU(TRF, color));

	verts.push_back(Vertex_PCU(BLF, color));
	verts.push_back(Vertex_PCU(TRF, color));
	verts.push_back(Vertex_PCU(TLF, color));

	//Left face
	verts.push_back(Vertex_PCU(BRB, color));
	verts.push_back(Vertex_PCU(BLF, color));
	verts.push_back(Vertex_PCU(TLF, color));

	verts.push_back(Vertex_PCU(BRB, color));
	verts.push_back(Vertex_PCU(TLF, color));
	verts.push_back(Vertex_PCU(TRB, color));

	// Back face
	verts.push_back(Vertex_PCU(BLB, color));
	verts.push_back(Vertex_PCU(BRB, color));
	verts.push_back(Vertex_PCU(TRB, color));

	verts.push_back(Vertex_PCU(BLB, color));
	verts.push_back(Vertex_PCU(TRB, color));
	verts.push_back(Vertex_PCU(TLB, color));

	// Right face
	verts.push_back(Vertex_PCU(BRF, color));
	verts.push_back(Vertex_PCU(BLB, color));
	verts.push_back(Vertex_PCU(TLB, color));

	verts.push_back(Vertex_PCU(BRF, color));
	verts.push_back(Vertex_PCU(TLB, color));
	verts.push_back(Vertex_PCU(TRF, color));

	// Top face
	verts.push_back(Vertex_PCU(TLF, color));
	verts.push_back(Vertex_PCU(TRF, color));
	verts.push_back(Vertex_PCU(TLB, color));

	verts.push_back(Vertex_PCU(TLF, color));
	verts.push_back(Vertex_PCU(TLB, color));
	verts.push_back(Vertex_PCU(TRB, color));

	// Bottom face
	verts.push_back(Vertex_PCU(BRB, color));
	verts.push_back(Vertex_PCU(BLB, color));
	verts.push_back(Vertex_PCU(BRF, color));

	verts.push_back(Vertex_PCU(BRB, color));
	verts.push_back(Vertex_PCU(BRF, color));
	verts.push_back(Vertex_PCU(BLF, color));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 displacement = end - start;
	Vec2 iBasis = displacement.GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	float halfThickness = 0.5f * thickness;
	Vec2 SL = start + jBasis * halfThickness;
	Vec2 SR = start - jBasis * halfThickness;
	Vec2 EL = end + jBasis * halfThickness;
	Vec2 ER = end - jBasis * halfThickness;

	verts.push_back(Vertex_PCU(SL, color));
	verts.push_back(Vertex_PCU(SR, color));
	verts.push_back(Vertex_PCU(ER, color));

	verts.push_back(Vertex_PCU(SL, color));
	verts.push_back(Vertex_PCU(ER, color));
	verts.push_back(Vertex_PCU(EL, color));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
{
	Vec2 const& start = lineSegment.m_start;
	Vec2 const& end = lineSegment.m_end;

	AddVertsForLineSegment2D(verts, start, end, thickness, color);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 color)
 {
	 constexpr int NUM_SIDES = 110;
	 float const degreesPerSide = 360.0f / static_cast<float>(NUM_SIDES);

	 for (int i = 0; i < NUM_SIDES; ++i)
	 {
		 float degrees = i * degreesPerSide;
		 Vec2 pointOnCircle = center + Vec2::MakeFromPolarDegrees(degrees, radius);

		 verts.push_back(Vertex_PCU(center, color));
		 verts.push_back(Vertex_PCU(pointOnCircle, color));

		 degrees += degreesPerSide;
		 pointOnCircle = center + Vec2::MakeFromPolarDegrees(degrees, radius);

		 verts.push_back(Vertex_PCU(pointOnCircle, color));
	 }
 }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2D const& box, Rgba8 const& color)
{
	Vec2 const& center = box.m_center;
	Vec2 const& iBasis = box.m_iBasisNormal.GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	Vec2 rightDisp = iBasis * box.m_halfDimensions.x;
	Vec2 upDisp = jBasis * box.m_halfDimensions.y;

	Vec2 TR = center + rightDisp + upDisp;
	Vec2 TL = center - rightDisp + upDisp;
	Vec2 BL = center - rightDisp - upDisp;
	Vec2 BR = center + rightDisp - upDisp;

	verts.push_back(Vertex_PCU(BL, color));
	verts.push_back(Vertex_PCU(BR, color));
	verts.push_back(Vertex_PCU(TR, color));

	verts.push_back(Vertex_PCU(BL, color));
	verts.push_back(Vertex_PCU(TR, color));
	verts.push_back(Vertex_PCU(TL, color));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

void AddvertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/, int numSlices /*= 8*/)
{
	UNUSED(UVs);
	float degreesPerSlice = 360.f / (float)numSlices;

	for (int i = 0; i < numSlices; i++)
	{
		//Bottom Disc
		Vec3 bottomLeft =  start  + Vec3(0.f, CosDegrees(i * degreesPerSlice ) * radius, SinDegrees(i * degreesPerSlice) * radius);
		Vec3 topLeft = start + Vec3(0.f, CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius);

		verts.push_back(Vertex_PCU(start, color));
		verts.push_back(Vertex_PCU(topLeft, color));
		verts.push_back(Vertex_PCU(bottomLeft, color));

		//Top Disc
 		Vec3 bottomRight = end + Vec3(0.f, CosDegrees(i * degreesPerSlice), SinDegrees(i * degreesPerSlice));
 		Vec3 topRight = end + Vec3(0.f, CosDegrees((i + 1) * degreesPerSlice), SinDegrees((i + 1) * degreesPerSlice));
 
 		verts.push_back(Vertex_PCU(end, color));
 		verts.push_back(Vertex_PCU(bottomRight, color));
 		verts.push_back(Vertex_PCU(topRight, color));

		//body
		verts.push_back(Vertex_PCU(topLeft, color));
		verts.push_back(Vertex_PCU(topRight, color));
		verts.push_back(Vertex_PCU(bottomRight, color));

		verts.push_back(Vertex_PCU(topLeft, color));
		verts.push_back(Vertex_PCU(bottomRight, color));
		verts.push_back(Vertex_PCU(topRight, color));
	}
	
	// Calculate the forward vector and normalize it
	Vec3 forward = end - start;
	forward.Normalize();

	// Calculate the new up vector using cross product and normalize it
	Vec3 newUp = CrossProduct3D(Vec3(1.f, 0.f, 0.f), forward).GetNormalized();

	// Calculate the left vector using cross product and normalize it
	Vec3 left = CrossProduct3D(forward, newUp).GetNormalized();

	// Calculate the translation necessary to put the cylinder between the start and end points
	Vec3 translation = (start + end) * 0.5f;

	// Create a new matrix with the calculated basis vectors and translation
	Mat44 transform;
	transform.SetIJKT3D(forward , left , newUp , translation);

	// Transform the vertex array using the new matrix
	TransformVertexArray3D(verts, transform);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForCylinder(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, int slices, Rgba8 const& color, AABB2 const& UVs)
{
	verts.reserve(size_t(8) * slices);

	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 worldIBasis = Vec3(1.0, 0.0f, 0.0f);
	Vec3 worldJBasis = Vec3(0.0f, 1.0, 0.0f);

	Vec3 jBasis, iBasis;

	if (fabsf(DotProduct3D(kBasis, worldIBasis)) < 1) {
		jBasis = CrossProduct3D(kBasis, worldIBasis).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else {
		jBasis = CrossProduct3D(kBasis, worldJBasis).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	iBasis *= radius;
	jBasis *= radius;

	float degDelta = 360.0f / static_cast<float>(slices);


	float prevCosYaw = 1.0f;
	float prevSinYaw = 0.0f;
	float leftU = UVs.m_mins.x;
	Vec2 midUVs = (UVs.m_maxs + UVs.m_mins) * 0.5f;

	for (float yaw = degDelta; yaw <= 360.0f; yaw += degDelta) {
		float sinYaw = SinDegrees(yaw);
		float cosYaw = CosDegrees(yaw);

		Vec3 bottomLeft = (iBasis * prevCosYaw) + (jBasis * prevSinYaw) + start;
		Vec3 bottomRight = (iBasis * cosYaw) + (jBasis * sinYaw) + start;

		Vec3 topLeft = (iBasis * prevCosYaw) + (jBasis * prevSinYaw) + end;
		Vec3 topRight = (iBasis * cosYaw) + (jBasis * sinYaw) + end;

		float rightU = RangeMapClamped(yaw, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);

		verts.emplace_back(bottomLeft, color, Vec2(leftU, UVs.m_mins.y));
		verts.emplace_back(bottomRight, color, Vec2(rightU, UVs.m_mins.y));
		verts.emplace_back(topRight, color, Vec2(rightU, UVs.m_maxs.y));

		verts.emplace_back(bottomLeft, color, Vec2(leftU, UVs.m_mins.y));
		verts.emplace_back(topRight, color, Vec2(rightU, UVs.m_maxs.y));
		verts.emplace_back(topLeft, color, Vec2(leftU, UVs.m_maxs.y));

		float leftTopU = midUVs.x + RangeMap(prevCosYaw, 0.0f, 1.0f, 0.0f, midUVs.x);
		float rightTopU = midUVs.x + RangeMap(cosYaw, 0.0f, 1.0f, 0.0f, midUVs.x);

		float leftTopV = midUVs.y + RangeMap(prevSinYaw, 0.0f, 1.0f, 0.0f, midUVs.y);
		float rightTopV = midUVs.y + RangeMap(sinYaw, 0.0f, 1.0f, 0.0f, midUVs.y);


		verts.emplace_back(start, color, midUVs);
		verts.emplace_back(bottomRight, color, Vec2(rightTopU, UVs.m_maxs.y - rightTopV));
		verts.emplace_back(bottomLeft, color, Vec2(leftTopU, UVs.m_maxs.y - leftTopV));

		verts.emplace_back(end, color, midUVs);
		verts.emplace_back(topLeft, color, Vec2(leftTopU, leftTopV));
		verts.emplace_back(topRight, color, Vec2(rightTopU, rightTopV));

		leftU = rightU;
		prevCosYaw = cosYaw;
		prevSinYaw = sinYaw;
	}

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddvertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/, int numSlices /*= 8*/)
{
	float degreesPerSlice = 360.f / (float)numSlices;

	Vec3 displacement = end - start;
	float height = displacement.GetLength();
	Vec3 top(0.f, 0.f, height);
	Vec3 bottomOrigin(0.f, 0.f, 0.f);

	for (int i = 0; i < numSlices; i++)
	{
		// Calculate polar UV coordinates for bottom and top discs
		float angle1 = i * degreesPerSlice;
		float angle2 = (i + 1) * degreesPerSlice;
		float u1 = 0.5f * (1.0f + cosf(angle1 * PI / 180.0f));
		float v1 = 0.5f * (1.0f + sinf(angle1 * PI / 180.0f));
		float u2 = 0.5f * (1.0f + cosf(angle2 * PI / 180.0f));
		float v2 = 0.5f * (1.0f + sinf(angle2 * PI / 180.0f));

		// Bottom Disc
		Vec3 bottomForward = bottomOrigin + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 bottomLeft = bottomOrigin + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);

		verts.push_back(Vertex_PCU(bottomOrigin, color, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(bottomLeft, color, Vec2(u2, v2)));
		verts.push_back(Vertex_PCU(bottomForward, color, Vec2(u1, v1)));

		// Top Disc
		Vec3 topForward = top + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 topLeft = top + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);

		verts.push_back(Vertex_PCU(top, color, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(topForward, color, Vec2(u1, v1)));
		verts.push_back(Vertex_PCU(topLeft, color, Vec2(u2, v2)));

		// Body
		float uBottomLeft = UVs.m_mins.x + (i * (UVs.m_maxs.x - UVs.m_mins.x) / numSlices);
		float uBottomRight = UVs.m_mins.x + ((i + 1) * (UVs.m_maxs.x - UVs.m_mins.x) / numSlices);

		verts.push_back(Vertex_PCU(topForward, color, Vec2(uBottomLeft, UVs.m_maxs.y)));
		verts.push_back(Vertex_PCU(bottomForward, color, Vec2(uBottomLeft, UVs.m_mins.y)));
		verts.push_back(Vertex_PCU(topLeft, color, Vec2(uBottomRight, UVs.m_maxs.y)));

		verts.push_back(Vertex_PCU(topLeft, color, Vec2(uBottomRight, UVs.m_maxs.y)));
		verts.push_back(Vertex_PCU(bottomForward, color, Vec2(uBottomLeft, UVs.m_mins.y)));
		verts.push_back(Vertex_PCU(bottomLeft, color, Vec2(uBottomRight, UVs.m_mins.y)));
	}

	
	Vec3 Kbasis = end - start;
	Kbasis.Normalize();

	Vec3 worldLeft(0.f, 1.f, 0.f);
	Vec3 worldTop(0.f, 0.f, 1.f);

	Vec3 Ibasis;

	Ibasis = CrossProduct3D(worldLeft, Kbasis);
	if (Ibasis == Vec3(0.f, 0.f, 0.f))
	{
		Ibasis = CrossProduct3D(Kbasis, worldTop);
	}
	Ibasis.Normalize();
	Vec3 Jbasis = CrossProduct3D(Kbasis, Ibasis);
	Jbasis.Normalize();
	Mat44 transformationMatrix;
	transformationMatrix.SetIJKT3D(Ibasis, Jbasis, Kbasis, start);

	TransformVertexArray3D(verts, transformationMatrix);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color /*= Rgba8(255, 255, 255, 255)*/)
{
	size_t startIndex = verts.size();
	float numSlices = 16.f;
	float degreesPerSlice = 360.f / numSlices;

	Vec3 displacement = end - start;
	float height = displacement.GetLength();
	Vec3 top(0.f, 0.f, height);
	Vec3 bottomOrigin(0.f, 0.f, 0.f);

	for (int i = 0; i < numSlices; i++)
	{
		//Bottom Disc
		Vec3 bottomForward = bottomOrigin + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 bottomLeft = bottomOrigin + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);
		verts.push_back(Vertex_PCU(bottomOrigin, color));
		verts.push_back(Vertex_PCU(bottomLeft, color));
		verts.push_back(Vertex_PCU(bottomForward, color));

		//Top Disc
		Vec3 topForward = top + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 topLeft = top + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);
		verts.push_back(Vertex_PCU(top, color));
		verts.push_back(Vertex_PCU(topForward, color));
		verts.push_back(Vertex_PCU(topLeft, color));

		// body
		verts.push_back(Vertex_PCU(topForward, color));
		verts.push_back(Vertex_PCU(bottomForward, color));
		verts.push_back(Vertex_PCU(topLeft, color));

		verts.push_back(Vertex_PCU(topLeft, color));
		verts.push_back(Vertex_PCU(bottomForward, color));
		verts.push_back(Vertex_PCU(bottomLeft, color));
	}


	Vec3 Kbasis = end - start;
	Kbasis.Normalize();

	Vec3 worldLeft(0.f, 1.f, 0.f);
	Vec3 worldTop(0.f, 0.f, 1.f);

	Vec3 Ibasis;
	//Vec3 Jbasis;

	Ibasis = CrossProduct3D(worldLeft, Kbasis);
	if (Ibasis == Vec3(0.f, 0.f, 0.f))
	{
		Ibasis = CrossProduct3D(Kbasis, worldTop);
	}
	Ibasis.Normalize();
	Vec3 Jbasis = CrossProduct3D(Kbasis, Ibasis);
	Jbasis.Normalize();
	Mat44 transformationMatrix;
	transformationMatrix.SetIJKT3D(Ibasis, Jbasis, Kbasis, start);

	int numVerticesAdded = (int)verts.size() - (int)startIndex;
	Vertex_PCU* firstVertexAdded = verts.data() + (int)startIndex;

	TransformVertexArray3D(numVerticesAdded, firstVertexAdded, transformationMatrix);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f,0.f),Vec2(1.f,1.f) ) */)
{
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(bottomleft, color, uvBL));
	verts.push_back(Vertex_PCU(bottomRight, color, uvBR));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));

	verts.push_back(Vertex_PCU(bottomleft, color, uvBL));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));
	verts.push_back(Vertex_PCU(topLeft, color, uvTL));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	Vec3 normal = CrossProduct3D((bottomRight - bottomleft).GetNormalized(), (topLeft - bottomleft).GetNormalized()).GetNormalized();

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PNCU(bottomleft, normal, color, uvBL));
	verts.push_back(Vertex_PNCU(bottomRight, normal, color, uvBR));
	verts.push_back(Vertex_PNCU(topRight, normal, color, uvTR));

	verts.push_back(Vertex_PNCU(bottomleft, normal, color, uvBL));
	verts.push_back(Vertex_PNCU(topRight, normal, color, uvTR));
	verts.push_back(Vertex_PNCU(topLeft, normal, color, uvTL));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	Vec3 midTop = (topLeft + topRight) * 0.5f;
	Vec3 midBottom = (bottomleft + bottomRight) * 0.5f;

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	Vec2 uvMidTop = (uvTL + uvTR) * 0.5f;
	Vec2 uvMidBottom = (uvBL + uvBR) * 0.5f;

	Vec3 leftNormal = (topLeft - topRight).GetNormalized();
	Vec3 rightNormal = -1 * leftNormal;
	Vec3 centerNormal = CrossProduct3D((bottomRight - bottomleft).GetNormalized(), (topLeft - bottomleft).GetNormalized()).GetNormalized();

	verts.push_back(Vertex_PNCU(bottomleft, leftNormal, color, uvBL));
	verts.push_back(Vertex_PNCU(midBottom, centerNormal, color, uvMidBottom));
	verts.push_back(Vertex_PNCU(midTop, centerNormal, color, uvMidTop));

	verts.push_back(Vertex_PNCU(bottomleft, leftNormal, color, uvBL));
	verts.push_back(Vertex_PNCU(midTop, centerNormal, color, uvMidTop));
	verts.push_back(Vertex_PNCU(topLeft, leftNormal, color, uvTL));

	verts.push_back(Vertex_PNCU(midBottom, centerNormal, color, uvMidBottom));
	verts.push_back(Vertex_PNCU(bottomRight, rightNormal, color, uvBR));
	verts.push_back(Vertex_PNCU(topRight, rightNormal, color, uvTR));

	verts.push_back(Vertex_PNCU(midBottom, centerNormal, color, uvMidBottom));
	verts.push_back(Vertex_PNCU(topRight, rightNormal, color, uvTR));
	verts.push_back(Vertex_PNCU(midTop, centerNormal, color, uvMidTop));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForIndexQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	unsigned int vertOffset = (unsigned int)verts.size();
	
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(bottomleft, color, uvBL));
	verts.push_back(Vertex_PCU(bottomRight, color, uvBR));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));
	verts.push_back(Vertex_PCU(topLeft, color, uvTL));
	
	indexes.push_back(vertOffset + 0);
	indexes.push_back(vertOffset + 1);
	indexes.push_back(vertOffset + 2);

	indexes.push_back(vertOffset + 0);
	indexes.push_back(vertOffset + 2);
	indexes.push_back(vertOffset + 3);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForIndexQuad3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	unsigned int vertOffset = (unsigned int)verts.size();

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	// Calculate normalized edge vectors for each vertex
	Vec3 normal = CrossProduct3D((bottomRight - bottomleft).GetNormalized(), (topLeft - bottomleft).GetNormalized()).GetNormalized();
	
	verts.push_back(Vertex_PNCU(bottomleft, normal, color, uvBL));
	verts.push_back(Vertex_PNCU(bottomRight, normal, color, uvBR));
	verts.push_back(Vertex_PNCU(topRight, normal, color, uvTR));
	verts.push_back(Vertex_PNCU(topLeft, normal, color, uvTL));

	indexes.push_back(vertOffset + 0);
	indexes.push_back(vertOffset + 1);
	indexes.push_back(vertOffset + 2);

	indexes.push_back(vertOffset + 0);
	indexes.push_back(vertOffset + 2);
	indexes.push_back(vertOffset + 3);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f , 1.f))*/, int numSlices /*= 8*/)
{
	size_t startIndex = verts.size();
	UNUSED(UVs);
	numSlices = 16;
	float degreesPerSlice = 360.f / (float)numSlices;

	Vec3 displacement = end - start;
	float height = displacement.GetLength();
	Vec3 coneTopa(0.f, 0.f, height);
	Vec3 bottomOrigin(0.f, 0.f, 0.f);

	for (int i = 0; i < numSlices; i++)
	{
		//Bottom Disc
		Vec3 bottomForward = bottomOrigin + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 bottomLeft = bottomOrigin + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);
		verts.push_back(Vertex_PCU(bottomOrigin, color));
		verts.push_back(Vertex_PCU(bottomLeft, color));
		verts.push_back(Vertex_PCU(bottomForward, color));
	}
	for (int i = 0; i < numSlices; i++)
	{
		Vec3 bottomForward = bottomOrigin + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 bottomLeft = bottomOrigin + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);
		verts.push_back(Vertex_PCU(coneTopa, color));
		verts.push_back(Vertex_PCU(bottomForward, color));
		verts.push_back(Vertex_PCU(bottomLeft, color));
	}

	Vec3 Kbasis = end - start;
	Kbasis.Normalize();

	Vec3 worldLeft(0.f, 1.f, 0.f);
	Vec3 worldTop(0.f, 0.f, 1.f);

	Vec3 Ibasis;

	// if kBasis not parallel with world left
	if (fabs(DotProduct3D(worldLeft, Kbasis)) < 1.f)
	{
		Ibasis = CrossProduct3D(worldLeft, Kbasis);
	}
	else // if kBasis PARALLEL with world left
	{
		Ibasis = CrossProduct3D(Kbasis, worldTop);
	}
	Ibasis.Normalize();
	Vec3 Jbasis = CrossProduct3D(Kbasis, Ibasis);
	Jbasis.Normalize();
	Vec3 translation = start;
	Mat44 transformationMatrix;
	transformationMatrix.SetIJKT3D(Ibasis, Jbasis, Kbasis, translation);

	TransformVertexArray3D((int)verts.size() - (int)startIndex, verts.data() + (int)startIndex, transformationMatrix);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForZCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, int slices /*= 16*/, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	verts.reserve(size_t(6) * slices);

	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 worldIBasis = Vec3(1.0, 0.0f, 0.0f);
	Vec3 worldJBasis = Vec3(0.0f, 1.0, 0.0f);

	Vec3 jBasis, iBasis;

	if (fabsf(DotProduct3D(kBasis, worldIBasis)) < 1) {
		jBasis = CrossProduct3D(kBasis, worldIBasis).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else {
		jBasis = CrossProduct3D(kBasis, worldJBasis).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	iBasis *= radius;
	jBasis *= radius;

	float degDelta = 360.0f / static_cast<float>(slices);

	float prevCosYaw = 1.0f;
	float prevSinYaw = 0.0f;
	float leftU = UVs.m_mins.x;
	Vec2 midUVs = (UVs.m_maxs + UVs.m_mins) * 0.5f;

	for (float yaw = degDelta; yaw <= 360.0f; yaw += degDelta) {
		float sinYaw = SinDegrees(yaw);
		float cosYaw = CosDegrees(yaw);

		Vec3 bottomLeft = (iBasis * prevCosYaw) + (jBasis * prevSinYaw) + start;
		Vec3 bottomRight = (iBasis * cosYaw) + (jBasis * sinYaw) + start;

		float rightU = RangeMapClamped(yaw, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);

		verts.emplace_back(bottomLeft, color, Vec2(leftU, UVs.m_mins.y));
		verts.emplace_back(bottomRight, color, Vec2(rightU, UVs.m_mins.y));
		verts.emplace_back(end, color, Vec2(rightU, UVs.m_maxs.y));

		float leftTopU = midUVs.x + RangeMap(prevCosYaw, 0.0f, 1.0f, 0.0f, midUVs.x);
		float rightTopU = midUVs.x + RangeMap(cosYaw, 0.0f, 1.0f, 0.0f, midUVs.x);

		float leftTopV = midUVs.y + RangeMap(prevSinYaw, 0.0f, 1.0f, 0.0f, midUVs.y);
		float rightTopV = midUVs.y + RangeMap(sinYaw, 0.0f, 1.0f, 0.0f, midUVs.y);


		verts.emplace_back(start, color, midUVs);
		verts.emplace_back(bottomRight, color, Vec2(rightTopU, UVs.m_maxs.y - rightTopV));
		verts.emplace_back(bottomLeft, color, Vec2(leftTopU, UVs.m_maxs.y - leftTopV));

		leftU = rightU;
		prevCosYaw = cosYaw;
		prevSinYaw = sinYaw;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts)
{
	AABB2 bounds; // Start with very large and very small values
	for (int i = 0; i < verts.size(); i++)
	{
		Vec2 pos2D = Vec2(verts[i].m_position.x, verts[i].m_position.y);
		bounds.StretchToIncludePoint(pos2D);
	}

	return bounds;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color /*= Rgba8::WHITE*/)
{
	float coneRadius = 2.f * radius;
	float coneHeight = 0.3f * (end - start).GetLength();
	Vec3 lineDirection = (end - start).GetNormalized();
	Vec3 coneBase = end - lineDirection * coneHeight;
	Vec3 coneTip = end;
	std::vector<Vertex_PCU> coneVerts;
	AddVertsForCone3D(verts, coneBase, coneTip, coneRadius, color);
	for (int coneVertsIndex = 0; coneVertsIndex < coneVerts.size(); coneVertsIndex++)
	{
		verts.push_back(coneVerts[coneVertsIndex]);
	}

	Vec3 cylinderEnd = coneBase;
	std::vector<Vertex_PCU> cylinderVerts;
	AddvertsForCylinderZ3D(verts, start, cylinderEnd, radius, color);
	for (int cylinderVertsIndex = 0; cylinderVertsIndex < cylinderVerts.size(); cylinderVertsIndex++)
	{
		verts.push_back(cylinderVerts[cylinderVertsIndex]);
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForNewArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, int slices /*= 16*/, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 worldIBasis = Vec3(1.0, 0.0f, 0.0f);
	Vec3 worldJBasis = Vec3(0.0f, 1.0, 0.0f);

	Vec3 jBasis, iBasis;

	if (fabsf(DotProduct3D(kBasis, worldIBasis)) < 1) {
		jBasis = CrossProduct3D(kBasis, worldIBasis).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else {
		jBasis = CrossProduct3D(kBasis, worldJBasis).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}

	float arrowLength = (end - start).GetLength();
	Vec3 arrowBodyEnd = start + (kBasis * arrowLength * 0.85f);
	AddVertsForCylinder(verts, start, arrowBodyEnd, radius, slices, color, UVs);
	AddVertsForZCone3D(verts, arrowBodyEnd, end, radius * 1.5f, slices, color, UVs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagonBorders2D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec2& center, float R, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = R - halfThickness;
	float outerRadius = R + halfThickness;
	constexpr int NUM_SIDES = 6;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	Vec3 normal(0.f, 0.f, 0.f);
	unsigned int vertOffset = static_cast<unsigned int>(verts.size());

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float degrees = DEGREES_PER_SIDE * (float)sideNum;
		float cosVal = CosDegrees(degrees);
		float sinVal = SinDegrees(degrees);
		Vec3 innerPos = Vec3(center.x + innerRadius * cosVal, center.y + innerRadius * sinVal, 0.f);
		Vec3 outerPos = Vec3(center.x + outerRadius * cosVal, center.y + outerRadius * sinVal, 0.f);

		verts.push_back(Vertex_PNCU(innerPos, normal, color, Vec2()));
		verts.push_back(Vertex_PNCU(outerPos, normal, color, Vec2()));
	}

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		int baseIndex = sideNum * 2;
		int nextBaseIndex = (sideNum + 1) % NUM_SIDES * 2; 

		// Triangle 1
		indexes.push_back(vertOffset + baseIndex);
		indexes.push_back(vertOffset + baseIndex + 1);
		indexes.push_back(vertOffset + nextBaseIndex);

		// Triangle 2
		indexes.push_back(vertOffset + nextBaseIndex);
		indexes.push_back(vertOffset + baseIndex + 1);
		indexes.push_back(vertOffset + nextBaseIndex + 1);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForSolidHexagon2D(std::vector<Vertex_PCU>& verts, const Vec2& center, float R, Rgba8 const& color)
{
	constexpr int NUM_SIDES = 6;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;

	Vec2 prevPos = center + Vec2::MakeFromPolarDegrees(0, R); 

	for (int sideNum = 1; sideNum <= NUM_SIDES; ++sideNum) 
	{
		float degrees = DEGREES_PER_SIDE * (float)sideNum;
		Vec2 currentPos = center + Vec2::MakeFromPolarDegrees(degrees, R);

		// Center vertex
		verts.push_back(Vertex_PCU(Vec3(center.x, center.y, 0.f), color, Vec2()));
		// Previous vertex on the hexagon perimeter
		verts.push_back(Vertex_PCU(Vec3(prevPos.x, prevPos.y, 0.f), color, Vec2()));
		// Current vertex on the hexagon perimeter
		verts.push_back(Vertex_PCU(Vec3(currentPos.x, currentPos.y, 0.f), color, Vec2()));

		prevPos = currentPos;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagonBorder2D_PCU(std::vector<Vertex_PCU>& verts, const Vec2& center, float R, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = R - halfThickness;
	float outerRadius = R + halfThickness;
	constexpr int NUM_SIDES = 6;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float degrees = DEGREES_PER_SIDE * (float)sideNum;
		float nextDegrees = DEGREES_PER_SIDE * (float)(sideNum + 1);

		// Calculate positions for the current and next vertices for both inner and outer points
		Vec3 innerPos = Vec3(center.x + innerRadius * CosDegrees(degrees), center.y + innerRadius * SinDegrees(degrees), 0.f);
		Vec3 outerPos = Vec3(center.x + outerRadius * CosDegrees(degrees), center.y + outerRadius * SinDegrees(degrees), 0.f);
		Vec3 innerPosNext = Vec3(center.x + innerRadius * CosDegrees(nextDegrees), center.y + innerRadius * SinDegrees(nextDegrees), 0.f);
		Vec3 outerPosNext = Vec3(center.x + outerRadius * CosDegrees(nextDegrees), center.y + outerRadius * SinDegrees(nextDegrees), 0.f);

		// Add vertices for the first triangle
		verts.push_back(Vertex_PCU(innerPos, color, Vec2()));
		verts.push_back(Vertex_PCU(outerPos, color, Vec2()));
		verts.push_back(Vertex_PCU(innerPosNext, color, Vec2()));

		// Add vertices for the second triangle
		verts.push_back(Vertex_PCU(innerPosNext, color, Vec2()));
		verts.push_back(Vertex_PCU(outerPos, color, Vec2()));
		verts.push_back(Vertex_PCU(outerPosNext, color, Vec2()));
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForLineSegmentUsingLinePrimitives(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, Rgba8 const& color /*= Rgba8(255, 255, 255, 255)*/)
{
	verts.push_back(Vertex_PCU(start, color, Vec2()));
	verts.push_back(Vertex_PCU(end, color, Vec2()));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForSphere3DNormal(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/)
{
	unsigned int indexOffset = (unsigned int)verts.size();
	std::vector<Vertex_PCUTBN> tempVerts;
	//Lets first draw the top half of the sphere
	float latitudeDegrees = 180.0f / numStacks;
	float longtitudeDegrees = 360.0f / numSlices;
	for (int i = 0; i < numStacks; i++)
	{
		for (int j = 0; j < numSlices; j++) 
		{
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * j, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * j, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * (j + 1), radius);
			
			Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			
			unsigned int numVertsSoFar = (unsigned int)tempVerts.size();
			
			indices.push_back(indexOffset + numVertsSoFar + 0);
			indices.push_back(indexOffset + numVertsSoFar + 1);
			indices.push_back(indexOffset + numVertsSoFar + 2);
			indices.push_back(indexOffset + numVertsSoFar + 0);
			indices.push_back(indexOffset + numVertsSoFar + 2);
			indices.push_back(indexOffset + numVertsSoFar + 3);
			
			tempVerts.push_back(Vertex_PCUTBN(BL, color, uvBL, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f), (BL - center).GetNormalized()));
			tempVerts.push_back(Vertex_PCUTBN(TR, color, uvBL, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f), (TR - center).GetNormalized()));
			tempVerts.push_back(Vertex_PCUTBN(TL, color, uvBL, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f), (TL - center).GetNormalized()));
			tempVerts.push_back(Vertex_PCUTBN(BR, color, uvBL, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f), (BR - center).GetNormalized()));
		}
	}
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
	CalculateTangentsAndBiTangents(verts, indices);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForNormalTangentSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const float& radius, const Vec3& center, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f , 1.f))*/, Rgba8 const& color /*= Rgba8(255, 255, 255, 255)*/)
{
	(void)UVs;
	int numLatitudeStacks = 16; 
	int numLongitudeSlices = 32; 

	// Top vertex
	Vec3 topPosition = center + Vec3(0.0f, 0.0f, radius);
	Vec3 topNormal = Vec3(0.0f, 0.0f, 1.0f);
	verts.push_back(Vertex_PCUTBN(topPosition, color, Vec2(0.0f, 0.0f), Vec3(), Vec3(), topNormal)); 
	int firstIndexAfterTop = 1;
	for (int longIndex = 0; longIndex < numLongitudeSlices; ++longIndex)
	{
		indexes.push_back(0);
		indexes.push_back(firstIndexAfterTop + longIndex);
		indexes.push_back(firstIndexAfterTop + (longIndex + 1) % numLongitudeSlices);
	}

	// Middle vertices
	for (int latIndex = 1; latIndex < numLatitudeStacks; ++latIndex)
	{
		float latRadians = PI * (float)latIndex / (float)numLatitudeStacks;
		for (int longIndex = 0; longIndex <= numLongitudeSlices; ++longIndex) 
		{
			float longRadians = 2.0f * PI * (float)longIndex / (float)numLongitudeSlices;
			Vec3 position = Vec3(radius * sinf(latRadians) * cosf(longRadians),
				radius * sinf(latRadians) * sinf(longRadians),
				radius * cosf(latRadians));
			Vec3 normal = position.GetNormalized();
			float u = (float)longIndex / (float)numLongitudeSlices;
			float v =  (float)latIndex / (float)numLatitudeStacks;

			verts.push_back(Vertex_PCUTBN(center + position, color, Vec2(u, v), Vec3(), Vec3(), normal));
		}
	}
	
	for (int latIndex = 0; latIndex < numLatitudeStacks - 2; ++latIndex)
	{
		int rowStartIndex = firstIndexAfterTop + latIndex * (numLongitudeSlices + 1);
		int nextRowStartIndex = rowStartIndex + numLongitudeSlices + 1;

		for (int longIndex = 0; longIndex < numLongitudeSlices; ++longIndex)
		{
			int nextLongIndex = (longIndex + 1) % numLongitudeSlices;

			indexes.push_back(rowStartIndex + longIndex);
			indexes.push_back(nextRowStartIndex + longIndex);
			indexes.push_back(rowStartIndex + nextLongIndex);

			indexes.push_back(rowStartIndex + nextLongIndex);
			indexes.push_back(nextRowStartIndex + longIndex);
			indexes.push_back(nextRowStartIndex + nextLongIndex);
		}
	}


	Vec3 bottomPosition = center - Vec3(0.0f, 0.0f, radius);
	Vec3 bottomNormal = Vec3(0.0f, 0.0f, -1.0f);
	verts.push_back(Vertex_PCUTBN(bottomPosition, color, Vec2(1.f, 1.0f), Vec3(), Vec3(), bottomNormal));


	int firstIndexBeforeBottom = firstIndexAfterTop + (numLatitudeStacks - 2) * (numLongitudeSlices + 1);
	int bottomIndex = (int)verts.size() - 1;
	for (int longIndex = 0; longIndex < numLongitudeSlices; ++longIndex)
	{
		indexes.push_back(bottomIndex);
		indexes.push_back(firstIndexBeforeBottom + (longIndex + 1) % numLongitudeSlices);
		indexes.push_back(firstIndexBeforeBottom + longIndex);
	}

	CalculateTangentsAndBiTangents(verts, indexes);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	// Draw the OBB part
	Vec2 boneDirection = (boneEnd - boneStart).GetNormalized();
	Vec2 boneOrthogonal = boneDirection.GetRotated90Degrees();
	Vec2 halfBoneWidth = boneOrthogonal * radius;

	Vec2 quadBL = boneStart + halfBoneWidth;
	Vec2 quadBR = boneStart - halfBoneWidth;
	Vec2 quadTL = boneEnd + halfBoneWidth;
	Vec2 quadTR = boneEnd - halfBoneWidth;

	// Push vertices for OBB in anti-clockwise order
	verts.push_back(Vertex_PCU(quadBL, color));
	verts.push_back(Vertex_PCU(quadBR, color));
	verts.push_back(Vertex_PCU(quadTR, color));

	verts.push_back(Vertex_PCU(quadBL, color));
	verts.push_back(Vertex_PCU(quadTR, color));
	verts.push_back(Vertex_PCU(quadTL, color));
	// Draw upper and lower hemispheres
	constexpr int NUM_SIDES = 32;
	float const degreesPerSide = 360.0f / static_cast<float>(NUM_SIDES);
	// Upper hemisphere
	for (int i = 0; i < NUM_SIDES; ++i)
	{
		float degrees = i * degreesPerSide;
		Vec2 pointOnCircle = boneEnd + Vec2::MakeFromPolarDegrees(degrees, radius);

		verts.push_back(Vertex_PCU(boneEnd, color));
		verts.push_back(Vertex_PCU(pointOnCircle, color));

		degrees += degreesPerSide;
		pointOnCircle = boneEnd + Vec2::MakeFromPolarDegrees(degrees, radius);

		verts.push_back(Vertex_PCU(pointOnCircle, color));
	}

	// Lower hemisphere
	for (int i = 0; i < NUM_SIDES; ++i)
	{
		float degrees = 360.0f + (i * degreesPerSide);
		Vec2 pointOnCircle = boneStart + Vec2::MakeFromPolarDegrees(degrees, radius);

		verts.push_back(Vertex_PCU(boneStart, color));
		verts.push_back(Vertex_PCU(pointOnCircle, color));

		degrees += degreesPerSide;
		pointOnCircle = boneStart + Vec2::MakeFromPolarDegrees(degrees, radius);

		verts.push_back(Vertex_PCU(pointOnCircle, color));
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color)
// {
// 	// Add vertices for the top hemisphere
// 	Vec2 topCenter = capsule.m_bone.m_end;
// 	float topRadius = capsule.radius;
// 	AddVertsForDisc2D(verts, topCenter, topRadius, color);
// 
// 	// Add vertices for the bottom hemisphere
// 	Vec2 bottomCenter = capsule.m_bone.m_start;
// 	float bottomRadius = capsule.radius;
// 	AddVertsForDisc2D(verts, bottomCenter, bottomRadius, color);
// 
// 	// Add vertices for the body of the capsule
// 	Vec2 const& boneStart = capsule.m_bone.m_start;
// 	Vec2 const& boneEnd = capsule.m_bone.m_end;
// 	Vec2 boneDir = (boneEnd - boneStart).GetNormalized();
// 	Vec2 boneLeftDir = boneDir.GetRotated90Degrees();
// 	Vec2 boneLeftStart = boneStart - boneLeftDir * capsule.radius;
// 	Vec2 boneLeftEnd = boneEnd - boneLeftDir * capsule.radius;
// 	Vec2 boneRightDir = -boneLeftDir;
// 	Vec2 boneRightStart = boneStart - boneRightDir * capsule.radius;
// 	Vec2 boneRightEnd = boneEnd - boneRightDir * capsule.radius;
// 	verts.push_back(Vertex_PCU(boneLeftStart, color));
// 	verts.push_back(Vertex_PCU(boneRightEnd, color));
// 	verts.push_back(Vertex_PCU(boneLeftEnd, color));
// 	verts.push_back(Vertex_PCU(boneRightEnd, color));
// 	verts.push_back(Vertex_PCU(boneRightStart, color));
// 	verts.push_back(Vertex_PCU(boneLeftStart, color));
// }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float lineThickness, float arrowRadius, Rgba8 const& color)
{
	// Line body
	AddVertsForLineSegment2D(verts, start, end, lineThickness, color);

	//left arrow
	Vec2 forwardDir = (end - start).GetNormalized();
	Vec2 leftDir = forwardDir.GetRotatedDegrees(135.f);

	Vec2 leftArrowStart = end;
	Vec2 leftArrowEnd = leftArrowStart + leftDir * arrowRadius;

	AddVertsForLineSegment2D(verts, leftArrowStart, leftArrowEnd, lineThickness, color);

	// Right arrow
	Vec2 rightDir = forwardDir.GetRotatedDegrees(225.f);

	Vec2 rightArrowStart = end;
	Vec2 rightArrowEnd = rightArrowStart + rightDir * arrowRadius;

	AddVertsForLineSegment2D(verts, rightArrowStart, rightArrowEnd, lineThickness, color);
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;
	constexpr int NUM_SIDES = 24;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float startDegrees = DEGREES_PER_SIDE * (float)sideNum;
		float endDegrees = DEGREES_PER_SIDE * (float)(sideNum + 1);
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);
		Vec3 innerStartPos = Vec3(center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f);
		Vec3 outerStartPos = Vec3(center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 outerEndPos = Vec3(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);
		Vec3 innerEndPos = Vec3(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;


		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexA].m_color = color;
		verts[vertIndexB].m_color = color;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
	};
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convexPoly, Rgba8 const& color)
{
// 	Vec2 middlePoint = convexPoly.GetCenter();
// 	Vertex_PCU middleVertex;
// 	middleVertex.m_color = color;
// 	middleVertex.m_position = Vec3(middlePoint);
// 
// 	// For the insides
// 	for (int pointIndex = 0; pointIndex + 1 < (int)convexPoly.m_ccwPoints.size(); pointIndex++)
// 	{
// 		Vec2 const& lineStart = convexPoly.m_ccwPoints[pointIndex];
// 		int nextIndex = pointIndex + 1;
// 		Vec2 const& lineEnd = convexPoly.m_ccwPoints[nextIndex];
// 
// 		verts.push_back(middleVertex);
// 		verts.push_back(Vertex_PCU(lineStart, color, Vec2(0.f, 0.f)));
// 		verts.push_back(Vertex_PCU(lineEnd, color, Vec2(0.f, 0.f)));
// 	}
// 
// 	Vec2 const& lastStart = convexPoly.m_ccwPoints[convexPoly.m_ccwPoints.size() - 1];
// 	Vec2 const& lastEnd = convexPoly.m_ccwPoints[0];
// 
// 	verts.push_back(middleVertex);
// 	verts.push_back(Vertex_PCU(lastStart, color, Vec2(0.f, 0.f)));
// 	verts.push_back(Vertex_PCU(lastEnd, color, Vec2(0.f, 0.f)));

	std::vector<Vec2> points = convexPoly.GetPoints();
	if (points.size() < 3)
	{
		ERROR_AND_DIE("Invalid Convex Polygon");
	}

	Vec3 end(points[points.size() - 1]);
	for (int index = 0; index < (int)points.size() - 2; index++)
	{
		Vec3 pos1(points[index]);
		Vec3 pos2(points[index + 1]);

		verts.emplace_back(pos1, color, Vec2::ZERO);
		verts.emplace_back(pos2, color, Vec2::ZERO);
		verts.emplace_back(end, color, Vec2::ZERO);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForWireConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convexPoly, Rgba8 const& borderColor, float lineThickness)
{
// 	for (int pointIndex = 0; pointIndex + 1 < (int)convexPoly.m_ccwPoints.size(); pointIndex++)
// 	{
// 		Vec2 const& lineStart = convexPoly.m_ccwPoints[pointIndex];
// 		int nextIndex = pointIndex + 1;
// 		Vec2 const& lineEnd = convexPoly.m_ccwPoints[nextIndex];
// 		LineSegment2 lineSegment;
// 		lineSegment.m_start = lineStart;
// 		lineSegment.m_end = lineEnd;
// 		AddVertsForLineSegment2D(verts, lineSegment, lineThickness, borderColor);
// 	}
// 
// 	Vec2 const& lastStart = convexPoly.m_ccwPoints[convexPoly.m_ccwPoints.size() - 1];
// 	Vec2 const& lastEnd = convexPoly.m_ccwPoints[0];
// 	LineSegment2 last;
// 	last.m_start = lastStart;
// 	last.m_end = lastEnd;
// 	AddVertsForLineSegment2D(verts, last, lineThickness, borderColor);



	std::vector<Vec2> points = convexPoly.GetPoints();
	if (points.size() < 3)
	{
		ERROR_AND_DIE("Invalid Convex Polygon");
	}

	for (int index = 0; index < (int)points.size() - 1; index++)
	{
		Vec2 const& pos1 = points[index];
		Vec2 const& pos2 = points[index + 1];
		LineSegment2 segment;
		segment.m_start = pos1;
		segment.m_end = pos2;
		AddVertsForLineSegment2D(verts, segment, lineThickness, borderColor);
	}
	LineSegment2 segment1;
	segment1.m_start = points[points.size() - 1];
	segment1.m_end = points[0];
	AddVertsForLineSegment2D(verts, segment1, lineThickness, borderColor);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForConvexHull2D(std::vector<Vertex_PCU>& verts, ConvexHull2D const& convexHull, Rgba8 const& color, float planeDrawDistance, float lineThickness)
{
	for (Plane2D const& plane : convexHull.m_boundingPlanes) 
	{
		Vec2 middlePoint = plane.m_planeNormal * plane.m_distance;
		Vec2 lineStart = middlePoint + plane.m_planeNormal.GetRotated90Degrees() * planeDrawDistance;
		Vec2 lineEnd = middlePoint - plane.m_planeNormal.GetRotated90Degrees() * planeDrawDistance;
		LineSegment2 lineSegment;
		lineSegment.m_start = lineStart;
		lineSegment.m_end = lineEnd;
		AddVertsForLineSegment2D(verts, lineSegment, lineThickness, color);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color /*= Rgba8(255, 255, 255, 255)*/, const AABB2& UVs /*= AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f))*/, int numLatitudeStacks /*= 8*/)
{
	int numLongitudeSlices = 2 * numLatitudeStacks;
	float degreesPerLatitudeStack = 180.f / (float)numLatitudeStacks;
	float degreesPerLongitudeSlice = 360.f / (float)numLongitudeSlices;

	for (int latIndex = 0; latIndex < numLatitudeStacks; latIndex++)
	{
		float latBottomDegrees = 90.f - (float)latIndex * degreesPerLatitudeStack;
		float latTopDegrees = 90.f - (float)(latIndex + 1) * degreesPerLatitudeStack;

		for (int longIndex = 0; longIndex < numLongitudeSlices; longIndex++)
		{
			float longLeftDegrees = (float)longIndex * degreesPerLongitudeSlice;
			float longRightDegrees = (float)(longIndex + 1) * degreesPerLongitudeSlice;

			Vec3 posBottomLeft = center + radius * Vec3::MakeFromPolarDegrees(latBottomDegrees, longLeftDegrees);
			Vec3 posBottomRight = center + radius * Vec3::MakeFromPolarDegrees(latBottomDegrees, longRightDegrees);
			Vec3 posTopLeft = center + radius * Vec3::MakeFromPolarDegrees(latTopDegrees, longLeftDegrees);
			Vec3 posTopRight = center + radius * Vec3::MakeFromPolarDegrees(latTopDegrees, longRightDegrees);

			float uLeft = UVs.m_mins.x + (UVs.m_maxs.x - UVs.m_mins.x) * (float)longIndex / (float)numLongitudeSlices;
			float uRight = UVs.m_mins.x + (UVs.m_maxs.x - UVs.m_mins.x) * (float)(longIndex + 1) / (float)numLongitudeSlices;
			float vTop = UVs.m_mins.y + (UVs.m_maxs.y - UVs.m_mins.y) * (float)(latIndex + 1) / (float)numLatitudeStacks; // invert vTop and vBottom
			float vBottom = UVs.m_mins.y + (UVs.m_maxs.y - UVs.m_mins.y) * (float)latIndex / (float)numLatitudeStacks;

			verts.push_back(Vertex_PCU(posBottomLeft, color, Vec2(uLeft, vBottom)));
			verts.push_back(Vertex_PCU(posBottomRight, color, Vec2(uRight, vBottom)));
			verts.push_back(Vertex_PCU(posTopRight, color, Vec2(uRight, vTop)));

			verts.push_back(Vertex_PCU(posBottomLeft, color, Vec2(uLeft, vBottom)));
			verts.push_back(Vertex_PCU(posTopRight, color, Vec2(uRight, vTop)));
			verts.push_back(Vertex_PCU(posTopLeft, color, Vec2(uLeft, vTop)));
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, const Rgba8& tint /*= Rgba8(255, 255, 255, 255)*/)
{
	(float)lineThickness;
	
	float degreesPerSlice = 360.f / (float)numSlices;
	Vec3 bottom(centerXY.x, centerXY.y, minMaxZ.m_min);
	Vec3 top(centerXY.x, centerXY.y, minMaxZ.m_max);

	for (int i = 0; i < numSlices; i++)
	{
		// Calculate the bottom and top positions for each slice
		float angle = i * degreesPerSlice;
		float nextAngle = (i + 1) * degreesPerSlice;
		
		// Bottom disc
		Vec3 bottomForward = bottom + Vec3(CosDegrees(angle) * radius, SinDegrees(angle) * radius, 0.f);
		Vec3 bottomLeft = bottom + Vec3(CosDegrees(nextAngle) * radius, SinDegrees(nextAngle) * radius, 0.f);

		verts.push_back(Vertex_PCU( bottom, tint));
		verts.push_back(Vertex_PCU(bottomLeft, tint));
		verts.push_back(Vertex_PCU(bottomForward, tint));
		
		// Top disc
		Vec3 topForward = top + Vec3(CosDegrees(i * degreesPerSlice) * radius, SinDegrees(i * degreesPerSlice) * radius, 0.f);
		Vec3 topLeft = top + Vec3(CosDegrees((i + 1) * degreesPerSlice) * radius, SinDegrees((i + 1) * degreesPerSlice) * radius, 0.f);

		verts.push_back(Vertex_PCU(top, tint));
		verts.push_back(Vertex_PCU(topForward, tint));
		verts.push_back(Vertex_PCU(topLeft, tint));


		// body
		verts.push_back(Vertex_PCU(topForward, tint));
		verts.push_back(Vertex_PCU(bottomForward, tint));
		verts.push_back(Vertex_PCU(topLeft, tint));

		verts.push_back(Vertex_PCU(topLeft, tint));
		verts.push_back(Vertex_PCU(bottomForward, tint));
		verts.push_back(Vertex_PCU(bottomLeft, tint));
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------

