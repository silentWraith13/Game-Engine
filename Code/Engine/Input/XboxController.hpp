#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum XboxButtonID
{
	XBOX_BUTTON_A,
	XBOX_BUTTON_B,
	XBOX_BUTTON_X,
	XBOX_BUTTON_Y,
	XBOX_BUTTON_UP,
	XBOX_BUTTON_DOWN, 
	XBOX_BUTTON_LEFT,
	XBOX_BUTTON_RIGHT,
	XBOX_BUTTON_START,
	XBOX_BUTTON_BACK,
	XBOX_BUTTON_L_SHOULDER,
	XBOX_BUTTON_R_SHOULDER,
	XBOX_BUTTON_L_THUMB,
	XBOX_BUTTON_R_THUMB,
	NUM = 14,
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class XboxController
{
	friend class InputSystem;
public:
	XboxController();
	~XboxController();

	bool					IsConnected();
	int						GetControllerID() const;
	AnalogJoystick const&	GetLeftStick() const;
	AnalogJoystick const&	GetRightStick() const;
	float					GetLeftTrigger() const;
	float					GetRightTrigger() const;
	KeyButtonState const&	GetButton(XboxButtonID buttonID) const;
	bool					IsButtonDown(XboxButtonID buttonID) const;
	bool					WasButtonJustPressed(XboxButtonID buttonID) const;
	bool					WasButtonJustReleased(XboxButtonID buttonID) const;
	void					SetVibration(int leftMotorSpeed, int rightMotorSpeed) const;

private:
	void					Update();
	void					Reset();
	void					UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY);
	void					UpdateTrigger(float& out_triggerValue, unsigned char rawValue);
	void					UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag);

private:
	int				m_id = -1;
	bool			m_isConnected = false;
	float			m_leftTrigger = 0.f;
	float			m_rightTrigger = 0.f;
	float			m_triggerInnerDeadZoneFraction = 0.05f;
	float			m_triggerOuterDeadZoneFraction = 0.90f;
	KeyButtonState	m_buttons[(int)XboxButtonID::NUM];
	AnalogJoystick	m_leftStick;
	AnalogJoystick	m_rightStick;
	float			m_stickInnerDeadZoneFraction = 0.3f;
	float			m_stickOuterDeadZoneFraction = 0.95f;
	float			m_vibration_time = 0.f;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------