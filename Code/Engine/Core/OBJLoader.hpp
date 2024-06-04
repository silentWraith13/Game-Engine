#pragma once
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class OBJLoader
{
public:
	static void ImportFromOBJFile(const std::string& filename, Mat44 fixupTransform, std::vector<Vertex_PCUTBN>& verts, std::vector< unsigned int>& indexes);
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------