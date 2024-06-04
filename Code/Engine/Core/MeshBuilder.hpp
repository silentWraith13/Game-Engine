#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/StringUtils.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct MeshImportOptions
{
	Mat44 m_transform;
	float m_scale = 1.0f;
	bool  m_reverseWindingOrder = false;
	bool  m_invertV = false;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class MeshBuilder
{
public:
	~MeshBuilder();
public:
	bool		ImportFromOBJFile(char const* m_fileName, MeshImportOptions& importOptions);
	bool		Save();
	bool		Load();
	bool		Load(std::string name);
	void		ApplyMeshOptions();
	void		ApplyInvertVTexture();
	void		ApplyTransform(Mat44 const& transform);
	void		ReverseWindingOrder();
	bool		UpdateFromBuilder(MeshBuilder const& builder);

	std::string GetFilePath();
public:
	std::string		m_filePath = "";
	std::string		m_texturePath = "";
	std::string		m_modelName = "";
	MeshImportOptions  m_importOptions;
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;

};