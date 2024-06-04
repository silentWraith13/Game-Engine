#pragma once
#include <Engine/Renderer/Shader.hpp>
#include <Engine/Renderer/Texture.hpp>

class Material
{
	Shader* m_shader = nullptr;
	Texture* m_diffuse = nullptr;
};