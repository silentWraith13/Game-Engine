#pragma once
#include <vector>


//--------------------------------------------------------------------------------------------------------------------------------------------------------
//Hierarchal clock that inherits time scale. Parent clocks scaled delta seconds down to child clocks to be used as their base delta seconds.
//Child clocks in turn scale that time and pass that down to their children. There is one system clock at the root of the hierarchy.
class Clock 
{
public:
	//Default constructor, uses the system clock as the parent of the new clock.
	Clock();

	//Constructor to specify a parent clock for the new clock
	explicit Clock(Clock& parent);

	//Destructor, unparents ourself and our children to avoid crashes but does not otherwise try to fix up the clock hierarchy.
	//That is the responsibility of the user of this class.
	~Clock();
	Clock(const Clock& copy) = delete;

	//Reset all book keeping variables values back to zero and then get the current time as the last updated time.
	void Reset();

	bool IsPaused() const;
	void Pause();
	void Unpause();
	void TogglePause();

	//Unpause for frame then pause again the next frame.
	void StepSingleFrame();

	//Set and get the value by which this clock scales delta seconds.
	void SetTimeScale(float timeScale);
	float GetTimeScale() const;

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	size_t GetFrameCount() const;

public:
	//Returns a reference to a static system clock that by default will be the parent of all other clocks if a parent is not specified.
	static Clock& GetSytemClock();

	//Called in BeginFrame to Tick the system clock, which will in turn advance the system clock,
	//which will in turn advance all of its children, thus updating the entire hierarchy.
	static void TickSytemClock();

protected:
	//Calculates the current delta seconds then calls Advance, passing down the delta seconds.
	void Tick();

	//Calculates delta seconds based on pausing and time scale, updates all book keeping variables,
	//calls Advance on all child locks and passes down our delta seconds, and handles pausing
	//after frames for stepping single frame.
	void Advance(float deltaSeconds);

	//Add a child clock as one of our children. Does not handles cases where child clock already has a parent.
	void AddChild(Clock* childClock);

	//Removes a child clock from our children if it is a child, otherwise does nothing.
	void RemoveChild(Clock* childClock);

protected:
	//Parent clock. will be nullptr for the root clock.
	Clock* m_parent = nullptr;

	//Add children of this clock.
	std::vector<Clock*> m_children;

	//Book keeping variables.
	float m_lastUpdateTimeInSeconds = 0.f;
	float m_totalSeconds = 0.f;
	float m_deltaSeconds = 0.f;
	size_t m_frameCount = 0;

	//Time scale for this clock.
	float m_timeScale = 1.0f;

	//Pauses the clock completely.
	bool m_isPaused = false;

	//For single stepping frames.
	bool m_stepSingleFrame = false;

	//Max delta time. Useful for preventing large time steps when stepping in a debugger.
	float m_maxDeltaSeconds = 0.1f;
};