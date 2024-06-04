#pragma once

class KeyButtonState {

public:
	bool IsKeyDown() const;
	bool WasKeyJustPressed() const;
	bool WasKeyJustReleased() const;
	void UpdateKeyDownLastFrame();
public:
	bool m_keyDownLastFrame = false;
	bool m_keyDownThisFrame = false;
};