#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>
#include "Engine/Math/OBB2D.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vec2;
struct OBB2D;
struct Vec3;
struct Rgba8;
struct AABB3;
struct LineSegment2;
struct Capsule2;
struct FloatRange;
struct Mat44;
class ConvexPoly2D;
class ConvexHull2D;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PNCU>& verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform);
void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, const Mat44& transform );
void RotateVertexArrayAroundCenterXY3D(int numVerts, Vertex_PCU* verts, float rotationDegreesAboutZ, Vec2 const& centerOfRotation);
AABB2 CalculateBounds(const std::vector<Vertex_PCU>& verts);
void CalculateTangentsAndBiTangents(std::vector<Vertex_PCUTBN>& vertices, const std::vector<unsigned int>& indices);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//2-D
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForHollowAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float thickness, Rgba8 const& color = Rgba8::WHITE, Rgba8 const& innerColor = Rgba8::BLACK, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 color);
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2D const& box, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
// void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float lineThickness, float arrowRadius, Rgba8 const& color);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convexPoly, Rgba8 const& color);
void AddVertsForWireConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convexPoly, Rgba8 const& borderColor, float lineThickness = 0.5f);
void AddVertsForConvexHull2D(std::vector<Vertex_PCU>& verts, ConvexHull2D const& convexHull, Rgba8 const& color, float planeDrawDistance, float lineThickness = 0.5f);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//3-D
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8(255, 255, 255), 
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f) ));

void AddVertsForIndexedQuadPCUTBN(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexedVerts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, 
	const Rgba8& color = Rgba8(255, 255, 255, 255), const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForIndexedCube(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color = Rgba8(255, 255, 255, 255),
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForAABBZ3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, 
	AABB2 const& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color = Rgba8(255, 255, 255, 255), 
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), int numLatitudeStacks = 8);

void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness,
	const Rgba8& tint = Rgba8(255, 255, 255, 255));

void AddVertsForCylinder(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, int slices = 16, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);

void AddvertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), int numSlices = 8);

void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8(255, 255, 255, 255));

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8(255, 255, 255, 255), 
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8(255, 255, 255, 255),
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8(255, 255, 255, 255),
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForIndexQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes,const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8(255, 255, 255, 255),
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForIndexQuad3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomleft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8(255, 255, 255, 255),
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color = Rgba8::WHITE, 
	const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f , 1.f)), int numSlices = 8);
void AddVertsForZCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, int slices = 16, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts);

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color = Rgba8::WHITE);
void AddVertsForNewArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, int slices = 16, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForHexagonBorders2D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec2& center, float R, float thickness, Rgba8 const& color);
void AddVertsForSolidHexagon2D(std::vector<Vertex_PCU>& verts, const Vec2& center, float R, Rgba8 const& color);

void AddVertsForHexagonBorder2D_PCU(std::vector<Vertex_PCU>& verts, const Vec2& center, float R, float thickness, Rgba8 const& color);
void AddVertsForNormalTangentSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const float& radius, const Vec3& center, const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), Rgba8 const& color = Rgba8(255, 255, 255, 255));
void AddVertsForLineSegmentUsingLinePrimitives(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, Rgba8 const& color = Rgba8(255, 255, 255, 255));
void AddVertsForSphere3DNormal(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
//--------------------------------------------------------------------------------------------------------------------------------------------------------