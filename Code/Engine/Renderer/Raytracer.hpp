#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Ray.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Raytracer
{
public:
	Ray m_currentRay;
	int m_bounces;

public:
	Rgba8 RayHitColor(Ray ray);

};