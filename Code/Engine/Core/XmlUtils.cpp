#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	XmlAttribute const* attribute = element.FindAttribute(attributeName);
	if (!attribute)
	{
		return defaultValue;
	}
	int value;
	attribute->QueryIntValue(&value);
	return value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	char attributeActualVal = defaultValue;
	if (attributeTextValue)
	{
		attributeActualVal = *attributeTextValue;
	}
	return attributeActualVal;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	bool value = element.BoolAttribute(attributeName, defaultValue);
	return value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	float value = element.FloatAttribute(attributeName, defaultValue);
	return value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Rgba8 color = defaultValue;
	if (attributeTextValue)
	{
		color.SetFromText(attributeTextValue);
	}
	return color;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Vec2 val = defaultValue;
	if (attributeTextValue)
	{
		val.SetFromText(attributeTextValue);
	}
	return val;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	IntVec2 val = defaultValue;
	if (attributeTextValue)
	{
		val.SetFromText(attributeTextValue);
	}
	return val;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	char const* valueAsText = element.Attribute(attributeName);
	std::string value = defaultValue;
	if (valueAsText)
	{
		value = valueAsText;
	}
	return value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
{
	char const* valueAsText = element.Attribute(attributeName);
	Strings attributeActualVal = defaultValues;
	if (valueAsText)
	{
		attributeActualVal = SplitStringOnDelimiter(valueAsText, ',');
	}
	return attributeActualVal;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	char const* valueAsText = element.Attribute(attributeName);
	std::string value = defaultValue;
	if (valueAsText)
	{
		value = valueAsText;
	}
	return value;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	FloatRange val = defaultValue;
	if (attributeTextValue)
	{
		val.SetFromText(attributeTextValue);
	}
	return val;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Vec3 val = defaultValue;
	if (attributeTextValue)
	{
		val.SetFromText(attributeTextValue);
	}
	return val;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	EulerAngles val = defaultValue;
	if (attributeTextValue)
	{
		val.SetFromText(attributeTextValue);
	}
	return val;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

