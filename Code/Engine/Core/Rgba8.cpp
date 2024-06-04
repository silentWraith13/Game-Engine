#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <sstream>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const Rgba8 Rgba8::WHITE = Rgba8(255, 255, 255, 255);
const Rgba8 Rgba8::GREY = Rgba8(128, 128, 128, 255);
const Rgba8 Rgba8::BLACK = Rgba8(0, 0, 0, 255);
const Rgba8 Rgba8::NOTHING = Rgba8(0, 0, 0, 0);
const Rgba8 Rgba8::BLACKLOWALPHA = Rgba8(0, 0, 0, 150);
const Rgba8 Rgba8::RED = Rgba8(255, 0, 0, 255);
const Rgba8 Rgba8::LIGHTRED = Rgba8(150, 0, 0, 255);
const Rgba8 Rgba8::GREEN = Rgba8(0, 255, 0, 255);
const Rgba8 Rgba8::LIGHTGREEN = Rgba8(0, 150, 0, 255);
const Rgba8 Rgba8::BLUE = Rgba8(0, 0, 255, 255);
const Rgba8 Rgba8::YELLOW = Rgba8(255, 255, 0, 255);
const Rgba8 Rgba8::CYAN = Rgba8(0, 255, 255, 255);
const Rgba8 Rgba8::MAGENTA = Rgba8(255, 0, 255, 255);
const Rgba8 Rgba8::LIGHTBLUE = Rgba8(135, 206, 235, 255);
const Rgba8 Rgba8::SKYBLUE = Rgba8(8, 143, 143, 255);
const Rgba8 Rgba8::DARKBLUE = Rgba8(8, 143, 200, 255);
const Rgba8 Rgba8::GOLDBROWN = Rgba8(153, 101, 21, 255);
const Rgba8 Rgba8::DARKERBLUE = Rgba8(8, 50, 255, 255);
const Rgba8 Rgba8::ORANGE = Rgba8(242, 151, 53, 255);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8::~Rgba8()
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8::Rgba8()
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8::Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	:r(r),g(g),b(b),a(a)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Rgba8::SetFromText(char const* text)
{
	Strings string;
	string = SplitStringOnDelimiter(text, ',');
	r = static_cast<unsigned char>(atoi((string[0].c_str())));
	g = static_cast<unsigned char>(atoi((string[1].c_str())));
	b = static_cast<unsigned char>(atoi((string[2].c_str())));

	if (string.size() >= 4)
	{
		a = static_cast<unsigned char>(atoi((string[3].c_str())));
	}
	else
	{
		a = 255; // Set default alpha value if it's not provided in the input string
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Rgba8::GetAsFloats(float* colorAsFloats) const
{
	colorAsFloats[0] = r / 255.0f;
	colorAsFloats[1] = g / 255.0f;
	colorAsFloats[2] = b / 255.0f;
	colorAsFloats[3] = a / 255.0f;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 Rgba8::BlendColors(Rgba8 startColor, Rgba8 endColor, float blendFactor) const
{
	float newR = Interpolate(static_cast<float>(startColor.r), static_cast<float>(endColor.r), blendFactor);
	float newG = Interpolate(static_cast<float>(startColor.g), static_cast<float>(endColor.g), blendFactor);
	float newB = Interpolate(static_cast<float>(startColor.b), static_cast<float>(endColor.b), blendFactor);
	float newA = Interpolate(static_cast<float>(startColor.a), static_cast<float>(endColor.a), blendFactor);
	return Rgba8(static_cast<unsigned char>(newR), static_cast<unsigned char>(newG), static_cast<unsigned char>(newB), static_cast<unsigned char>(newA));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Rgba8::operator==(const Rgba8& compare) const
{
	return (r == compare.r && g == compare.g && b == compare.b && a == compare.a );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 Rgba8::operator*(float scale) const
{
	return Rgba8(
		static_cast<unsigned char>(r * scale),
		static_cast<unsigned char>(g * scale),
		static_cast<unsigned char>(b * scale),
		static_cast<unsigned char>(a * scale));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

