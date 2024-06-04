#pragma once
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include <map>
#include <string>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
struct IntVec2;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
typedef tinyxml2::XMLDocument       XmlDocument;
typedef tinyxml2::XMLElement        XmlElement;
typedef tinyxml2::XMLAttribute      XmlAttribute;
typedef tinyxml2::XMLError          XmlResult;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NamedStrings
{
public:
	NamedStrings() = default;
	~NamedStrings() = default;

	void			PopulateFromXmlElementAttributes(XmlElement const& element);
	bool			HasKey(std::string const& keyName) const;
	void			SetValue(std::string const& keyName, std::string const& newValue);
	std::string		GetValue(std::string const& keyName, std::string const& defaultValue) const;
	bool			GetValue(std::string const& keyName, bool defaultValue) const;
	int				GetValue(std::string const& keyName, int defaultValue) const;
	float			GetValue(std::string const& keyName, float defaultValue) const;
	std::string		GetValue(std::string const& keyName, char const* defaultValue) const;
	Rgba8			GetValue(std::string const& keyName, Rgba8 const& defaultValue) const;
	Vec2			GetValue(std::string const& keyName, Vec2 const& defaultValue) const;
	IntVec2			GetValue(std::string const& keyName, IntVec2 const& defaultValue) const;

protected:
	std::map< std::string, std::string >	m_keyValuePairs;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------