#pragma once

#include "Engine/Renderer/RaytracingHelpers.hpp"
#include <Engine/Math/Vec3.hpp>
#include <Engine/Math/Vec2.hpp>
#include <vector>
#include <random>
#include <functional>
struct AlignedHemisphereSample3D;

enum SamplerType
{
	Random,
	Cosine
};

class Sampler
{
	//Constructor 
	public:
		Sampler(SamplerType type, int numSamples, int numSets);
		Vec2 RandomFloat01_2D();
		UINT GetRandomNumber(UINT min, UINT max);
		~Sampler();
		std::function<UINT()> GetRandomJump;        // Generates a random uniform index within [0, m_numSamples - 1]
		std::function<UINT()> GetRandomSetJump;     // Generates a random uniform index within [0, m_numSampleSets - 1]
		std::function<float()> GetRandomFloat01;    // Generates a random uniform float within [0,1)
		std::function<float()> GetRandomFloat01inclusive; // Generates a random uniform float within [0,1]
		//UINT GetRandomJump();
		//UINT GetRandomSetJump();
		void Swap(int index1, int index2, bool x);
		UINT GetSampleIndex();

		//void GenerateSamples(StructuredBuffer<AlignedHemisphereSample3D> buffer);
	public:
		std::mt19937		m_generatorURNG;
		std::mt19937		m_generatorURNG2;
		std::vector<UINT>   m_shuffledIndices;
		UINT			    m_jump;
		UINT				m_setJump;
		UINT				m_index;
		SamplerType			m_type = SamplerType::Cosine;
		UINT			    m_numSamples = 0;
		UINT			    m_numSampleSets = 0;
		UINT				m_sampleIndex = 0;
		std::vector<Vec3>   m_hemisphereSampleData; 
		std::vector<Vec2>	m_sampleData;
};