#include "Engine/Input/KeyButtonState.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool KeyButtonState::IsKeyDown() const
{
	if (m_keyDownLastFrame && m_keyDownThisFrame)
	{
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool KeyButtonState::WasKeyJustPressed() const
{
	if (!m_keyDownLastFrame && m_keyDownThisFrame)
	{
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool KeyButtonState::WasKeyJustReleased() const
{
	if (m_keyDownLastFrame && !m_keyDownThisFrame)
	{
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void KeyButtonState::UpdateKeyDownLastFrame()
{
	m_keyDownLastFrame = m_keyDownThisFrame;
}
