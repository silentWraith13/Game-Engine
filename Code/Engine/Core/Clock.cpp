#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"

static Clock s_systemClock;

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Clock::Clock()
{
	if (this != &s_systemClock)
	{
		m_parent = &s_systemClock;
		s_systemClock.AddChild(this);
	}
}  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Clock::Clock(Clock& parent)
{
	m_parent = &parent;
	parent.AddChild(this);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::Reset()
{
	m_lastUpdateTimeInSeconds = static_cast<float>(GetCurrentTimeSeconds());		
	m_totalSeconds = 0.f;
	m_deltaSeconds = 0.f;
	m_frameCount = 0;
	m_timeScale = 1.0f;		
	m_isPaused = false;
	m_stepSingleFrame = false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Clock::IsPaused() const
{
	return m_isPaused;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::Pause()
{
	m_isPaused = true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::Unpause()
{
	m_isPaused = false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::StepSingleFrame()
{
	m_stepSingleFrame = true;
	Unpause();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Clock::GetTimeScale() const
{
	return m_timeScale;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

Clock& Clock::GetSytemClock()
{
	//Not sure how to do this.
	return s_systemClock;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::TickSytemClock()
{
	GetSytemClock().Tick();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::Tick()		
{
	float currentTimeInSeconds = static_cast<float>(GetCurrentTimeSeconds());		
	m_deltaSeconds = currentTimeInSeconds - m_lastUpdateTimeInSeconds;
	m_lastUpdateTimeInSeconds = currentTimeInSeconds;

	if (m_deltaSeconds > m_maxDeltaSeconds) {
		m_deltaSeconds = m_maxDeltaSeconds;
	}
	
	Advance(m_deltaSeconds);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::Advance(float deltaSeconds)
{
	if (m_isPaused)
		deltaSeconds = 0.f;
	
	// Scale the delta seconds by the time scale
	deltaSeconds *= m_timeScale;

	// Update the book keeping variables			
	m_deltaSeconds = deltaSeconds;
	m_totalSeconds += deltaSeconds;
	++m_frameCount;

	// Call Advance on all child clocks and pass down the delta seconds
	for (int i = 0; i < m_children.size(); i++)
	{
		Clock* child = m_children[i];
		child->Advance(deltaSeconds);
	}

	// Handle the case of stepping a single frame
	if (m_stepSingleFrame)
	{
		m_stepSingleFrame = false;
		m_isPaused = true;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::AddChild(Clock* childClock)
{
	m_children.push_back(childClock);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Clock::RemoveChild(Clock* childClock)
{
	auto it = std::find(m_children.begin(), m_children.end(), childClock);
	if (it != m_children.end())
	{
		m_children.erase(it);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Clock::~Clock()
{
	if (m_parent != nullptr)
	{
		m_parent->RemoveChild(this);
	}
	
	for (int i = 0; i < m_children.size(); i++)
	{
		Clock* child = m_children[i];
		child->m_parent = nullptr;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
