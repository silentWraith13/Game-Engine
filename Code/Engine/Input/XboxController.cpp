#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0")

//--------------------------------------------------------------------------------------------------------------------------------------------------------
XboxController::XboxController()
{
	m_leftStick.SetDeadZoneThresholds(m_stickInnerDeadZoneFraction, m_stickOuterDeadZoneFraction);
	m_rightStick.SetDeadZoneThresholds(m_stickInnerDeadZoneFraction, m_stickOuterDeadZoneFraction);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
XboxController::~XboxController()
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool XboxController::IsConnected()
{
	return m_isConnected;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int XboxController::GetControllerID() const
{
	return m_id;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[buttonID];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].IsKeyDown();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].WasKeyJustPressed();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].WasKeyJustReleased();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void XboxController::SetVibration(int leftMotorSpeed, int rightMotorSpeed) const
{
	WORD left = WORD(leftMotorSpeed);
	WORD right = WORD(rightMotorSpeed);
	XINPUT_VIBRATION* x_v = new XINPUT_VIBRATION();
	x_v->wLeftMotorSpeed = left;
	x_v->wRightMotorSpeed = right;
	XInputSetState(m_id, x_v);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus != ERROR_SUCCESS)
	{
		Reset();
		m_isConnected = false;
		return;
	}
	m_isConnected = true;

	XINPUT_GAMEPAD const& state = xboxControllerState.Gamepad;
	UpdateJoystick(m_leftStick, state.sThumbLX, state.sThumbLY);
	UpdateJoystick(m_rightStick, state.sThumbRX, state.sThumbRY);

	UpdateTrigger(m_leftTrigger, state.bLeftTrigger);
	UpdateTrigger(m_rightTrigger, state.bRightTrigger);

	UpdateButton(XBOX_BUTTON_A, state.wButtons, XINPUT_GAMEPAD_A);
	UpdateButton(XBOX_BUTTON_B, state.wButtons, XINPUT_GAMEPAD_B);
	UpdateButton(XBOX_BUTTON_X, state.wButtons, XINPUT_GAMEPAD_X);
	UpdateButton(XBOX_BUTTON_Y, state.wButtons, XINPUT_GAMEPAD_Y);
	UpdateButton(XBOX_BUTTON_UP, state.wButtons, XINPUT_GAMEPAD_DPAD_UP);
	UpdateButton(XBOX_BUTTON_DOWN, state.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
	UpdateButton(XBOX_BUTTON_LEFT, state.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
	UpdateButton(XBOX_BUTTON_RIGHT, state.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
	UpdateButton(XBOX_BUTTON_START, state.wButtons, XINPUT_GAMEPAD_START);
	UpdateButton(XBOX_BUTTON_BACK, state.wButtons, XINPUT_GAMEPAD_BACK);
	UpdateButton(XBOX_BUTTON_L_SHOULDER, state.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
	UpdateButton(XBOX_BUTTON_R_SHOULDER, state.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	UpdateButton(XBOX_BUTTON_L_THUMB, state.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
	UpdateButton(XBOX_BUTTON_R_THUMB, state.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void XboxController::Reset()
{
	m_id = -1;
	m_isConnected = false;
	m_leftTrigger = 0.f;
	m_rightTrigger = 0.f;
	m_leftStick.Reset();
	m_rightStick.Reset();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	out_joystick.UpdatePosition(static_cast<float>(rawX), static_cast<float>(rawY));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	float f_rawValue = static_cast<float>(rawValue);
	f_rawValue = GetClamped(f_rawValue, m_triggerInnerDeadZoneFraction * 256.f, m_triggerOuterDeadZoneFraction * 256.f);
	out_triggerValue = RangeMap(f_rawValue, m_triggerInnerDeadZoneFraction * 256.f, m_triggerOuterDeadZoneFraction * 256.f, 0.f, 1.f);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	m_buttons[buttonID].UpdateKeyDownLastFrame();
	if (buttonFlags & buttonFlag)
	{
		m_buttons[buttonID].m_keyDownThisFrame = true;
	}
	else
	{
		m_buttons[buttonID].m_keyDownThisFrame = false;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
