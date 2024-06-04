#include <stdlib.h>
#include "RandomNumberGenerator.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
RandomNumberGenerator::RandomNumberGenerator(unsigned int seed /*= 0*/)
	:m_seed(seed)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	unsigned int randomUint = Get1dNoiseUint(m_position++, m_seed);
	return randomUint % maxNotInclusive;
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	unsigned int range = maxInclusive - minInclusive + 1;
	int randomNumber = RollRandomIntLessThan(range);
	return randomNumber + minInclusive;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return Get1dNoiseZeroToOne(m_position++, m_seed);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float randomFloat = RollRandomFloatZeroToOne();
	return minInclusive + randomFloat * (maxInclusive - minInclusive);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool RandomNumberGenerator::RollRandomChance(float probabilityOfTrue)
{
	float randomFloat = RollRandomFloatZeroToOne();
	return randomFloat < probabilityOfTrue;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------