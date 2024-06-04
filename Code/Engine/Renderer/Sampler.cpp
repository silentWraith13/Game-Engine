#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include <numeric>
#include <algorithm>
#include <random>

Sampler::Sampler(SamplerType type, int numSamples, int numSets)
{
	RandomNumberGenerator rng;
	m_generatorURNG.seed(1729);
	m_type = type;
	m_numSamples = numSamples;
	m_numSampleSets = numSets;
	m_sampleData.resize(m_numSamples * m_numSampleSets, Vec2(FLT_MAX, FLT_MAX));
	m_shuffledIndices.resize(m_numSamples * m_numSampleSets);
	m_hemisphereSampleData.resize(m_numSamples * m_numSampleSets, Vec3(FLT_MAX, FLT_MAX, FLT_MAX));


	m_generatorURNG.seed(1729);
	m_generatorURNG2.seed(1879);

	std::uniform_int_distribution<UINT> jumpDistribution(0, m_numSamples - 1);
	std::uniform_int_distribution<UINT> jumpSetDistribution(0, m_numSampleSets - 1);

	std::uniform_real_distribution<float> unitSquareDistribution(0.f, 1.f);

	// Specify the next representable value for the end range, since
	// uniform_real_distribution constructs excluding the end value [being, end).
	std::uniform_real_distribution<float> unitSquareDistributionInclusive(0.f, nextafter(1.f, FLT_MAX));

	GetRandomJump = bind(jumpDistribution, ref(m_generatorURNG));
	GetRandomSetJump = bind(jumpSetDistribution, ref(m_generatorURNG));
	GetRandomFloat01 = bind(unitSquareDistribution, ref(m_generatorURNG));
	GetRandomFloat01inclusive = bind(unitSquareDistributionInclusive, ref(m_generatorURNG));

    for (UINT s = 0; s < m_numSampleSets; s++)
    {
        // Generate samples on 2 level grid, with one sample per each (x,y)
        UINT sampleSetStartID = s * m_numSamples;

        const UINT T = m_numSamples;
        const UINT N = static_cast<UINT>(sqrt(T));
		#define SAMPLE(i) m_sampleData[sampleSetStartID + i]

        // Generate random samples
        for (UINT col = 0, i = 0; col < N; col++)
            for (UINT row = 0; row < N; row++, i++)
            {
                Vec2 stratum = Vec2((float)row, (float)col);
				Vec2 cell = Vec2((float)col, (float)row);
				Vec2 randomValue = RandomFloat01_2D();
				SAMPLE(i).x = (randomValue.x + cell.x) / T + stratum.x / N;
				SAMPLE(i).y = (randomValue.y + cell.y) / T + stratum.y / N;
            }

        // Shuffle sample axes such that there's a sample in each stratum 
        // and n-rooks is maintained.

        // Shuffle x coordinate across rows within a column
        for (UINT row = 0; row < N - 1; row++)
            for (UINT col = 0; col < N; col++)
            {
                UINT k = GetRandomNumber(row + 1, N - 1);
				Swap(row * N + col, k * N + col,true);
            }

        // Shuffle y coordinate across columns within a row
        for (UINT row = 0; row < N; row++)
            for (UINT col = 0; col < N - 1; col++)
            {
                UINT k = GetRandomNumber(col + 1, N - 1);
				Swap(row * N + col, row * N + k, false);
            }

    }


	//for (UINT i = 0; i < m_numSamples * m_numSampleSets; i++)
	//{
	//	m_sampleData[i] = Vec2(rng.GetRandomFloatZeroToOne(), rng.GetRandomFloatZeroToOne());
	//}

	if (type == SamplerType::Cosine)
	{
        for (UINT i = 0; i < m_sampleData.size(); i++)
        {
            float cosTheta = powf((1.f - m_sampleData[i].y), 1.0f / (2.0f));
            float sinTheta = sqrtf(1.f - cosTheta * cosTheta);
            m_hemisphereSampleData[i].x = sinTheta * cosf(6.283185307f * m_sampleData[i].x);
			m_hemisphereSampleData[i].y = sinTheta * sinf(6.283185307f * m_sampleData[i].x);
			m_hemisphereSampleData[i].z = cosTheta;
        }
	}
	for (UINT i = 0; i < m_numSampleSets; i++)
	{
		auto first = begin(m_shuffledIndices) + i * m_numSamples;
		auto last = first + m_numSamples;

		iota(first, last, 0u); // Fill with 0, 1, ..., m_numSamples - 1 
		shuffle(first, last, m_generatorURNG);
	}
}

Vec2 Sampler::RandomFloat01_2D()
{
	return Vec2(GetRandomFloat01(), GetRandomFloat01());
}


UINT Sampler::GetRandomNumber(UINT min, UINT max)
{
	std::uniform_int_distribution<UINT> distribution(min, max);
	return distribution(m_generatorURNG);
}

//UINT Sampler::GetRandomJump()
//{
//	RandomNumberGenerator rng;
//	return rng.GetRandomIntLessThan(m_numSamples);
//}

void Sampler::Swap(int index1, int index2, bool xValue)
{
	if (xValue)
	{
		float tempx = m_sampleData[index1].x;
		m_sampleData[index1].x = m_sampleData[index2].x;
		m_sampleData[index2].x = tempx;
	}
	else
	{
		float tempy = m_sampleData[index1].y;
		m_sampleData[index1].y = m_sampleData[index2].y;
		m_sampleData[index2].y = tempy;
	}
}

//UINT Sampler::GetRandomSetJump()
//{
//	RandomNumberGenerator rng;
//	return rng.GetRandomIntLessThan(m_numSampleSets);
//}
UINT Sampler::GetSampleIndex()
{
	// Initialize sample and set jumps.
	if (m_index % m_numSamples == 0)
	{
		// Pick a random index jump within a set.
		m_jump = GetRandomJump();

		// Pick a random set index jump.
		m_setJump = GetRandomSetJump() * m_numSamples;
	}
	return m_setJump + m_shuffledIndices[(m_index++ + m_jump) % m_numSamples];
}

