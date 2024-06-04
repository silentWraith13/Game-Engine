#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attrib = element.FirstAttribute();
	while (attrib)
	{
		SetValue(attrib->Name(), attrib->Value());
		attrib = attrib->Next();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool NamedStrings::HasKey(std::string const& keyName) const
{
	auto found = m_keyValuePairs.find(keyName);
	return (found != m_keyValuePairs.end());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return found->second;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	std::string boolText = found->second;
	if (boolText == "true" || boolText == "True" || boolText == "TRUE")
	{
		return true;
	}
	else if (boolText == "false" || boolText == "False" || boolText == "FALSE")
	{
		return false;
	}
	else
	{
		ERROR_AND_DIE("Illegal bool string");
	}
	
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return static_cast<int>(atoi(found->second.c_str()));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return static_cast<float>(atof(found->second.c_str()));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end() )
	{
		return defaultValue;
	}
	return found->second;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	Rgba8 foundVec2;
	foundVec2.SetFromText(found->second.c_str());
	return foundVec2;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	Vec2 foundVec2;
	foundVec2.SetFromText(found->second.c_str());
	return foundVec2;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	IntVec2 foundVec2;
	foundVec2.SetFromText(found->second.c_str());
	return foundVec2;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
