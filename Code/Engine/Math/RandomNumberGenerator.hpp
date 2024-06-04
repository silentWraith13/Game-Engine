#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class RandomNumberGenerator 
{

public:
	RandomNumberGenerator(unsigned int seed = 0);
	int    RollRandomIntLessThan(int maxNotInclusive);
	int    RollRandomIntInRange(int minInclusive, int maxInclusive);
	float  RollRandomFloatZeroToOne();
	float  RollRandomFloatInRange(float minInclusive, float maxInclusive);
	bool   RollRandomChance(float probabilityOfTrue);
private:
 unsigned int            m_seed=0;   
 int                     m_position=0;             
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------