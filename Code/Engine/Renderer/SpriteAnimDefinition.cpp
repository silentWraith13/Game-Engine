#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//	
SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float secondsPerFrame /*= 20.f*/, SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::LOOP*/)
	:m_spriteSheet(sheet)
	,m_startSpriteIndex(startSpriteIndex)
	,m_endSpriteIndex(endSpriteIndex)
	,m_secondsPerFrame(secondsPerFrame)
	,m_playbackType(playbackType)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void SpriteAnimDefinition::SetStartEndIndex(int startIndex, int endIndex)
{
	m_startSpriteIndex = startIndex;
	m_endSpriteIndex = endIndex;
}

void SpriteAnimDefinition::SetPlaybackType(SpriteAnimPlaybackType animPlaybackType)
{
	m_playbackType = animPlaybackType;
}

void SpriteAnimDefinition::SetSecondsPerFrame(float secondsPerFrame)
{
	m_secondsPerFrame = secondsPerFrame;
}

const float SpriteAnimDefinition::GetDuration() const
{
	return m_secondsPerFrame;
}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	if (m_playbackType == SpriteAnimPlaybackType::ONCE)
	{
		int animFrame = RoundDownToInt(seconds / m_secondsPerFrame);
		int spriteIndex = GetClamped(m_startSpriteIndex + animFrame, m_startSpriteIndex, m_endSpriteIndex);
		return m_spriteSheet.GetSpriteDef(spriteIndex);
	}
	else if(m_playbackType == SpriteAnimPlaybackType::LOOP)
	{
		int numFrames = m_endSpriteIndex - m_startSpriteIndex + 1;
		int animFrames = RoundDownToInt(seconds / m_secondsPerFrame);
	
		return m_spriteSheet.GetSpriteDef(animFrames % numFrames + m_startSpriteIndex);
	}
	else if (m_playbackType == SpriteAnimPlaybackType::PINGPONG)
	{
		return m_spriteSheet.GetSpriteDef(m_startSpriteIndex); // TO-DO:Solve later
	}
	else
	{
		ERROR_AND_DIE("Unknown Sprite anim playback mode");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
