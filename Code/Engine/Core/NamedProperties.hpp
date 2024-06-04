#pragma once
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include <map>
#include <Engine/Math/Vec2.hpp>
#include <Engine/Math/IntVec2.hpp>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
typedef tinyxml2::XMLDocument       XmlDocument;
typedef tinyxml2::XMLElement        XmlElement;
typedef tinyxml2::XMLAttribute      XmlAttribute;
typedef tinyxml2::XMLError          XmlResult;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class HashedCaseInsensitiveString
{
public:
	explicit HashedCaseInsensitiveString(std::string const& value); 
	unsigned int CalculateHash(std::string const& text) const; 

	bool operator==(HashedCaseInsensitiveString const& compare) const;
	bool operator!=(HashedCaseInsensitiveString const& compare) const;
	bool operator==(std::string const& compareString) const;
	bool operator!=(std::string const& compareString) const;
	bool operator<(const HashedCaseInsensitiveString& other) const;
	
	std::string GetString() const 
	{
		return m_textCaseIntact;
	}

	std::string  m_textCaseIntact;
	unsigned int m_lowerCaseHash = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NamedPropertyBase
{
public:
	virtual ~NamedPropertyBase() = default;
	virtual NamedPropertyBase* GetClone() const = 0;
	virtual bool IsType(std::type_info const& type) const = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename T_Value>
class NamedProperty : public NamedPropertyBase
{
public:
	NamedPropertyBase* GetClone() const override
	{
		NamedProperty<T_Value>* clone = new NamedProperty<T_Value>(); 
		clone->m_data = m_data;
		return clone;
	}
	virtual bool IsType(std::type_info const& type) const override { return typeid(m_data) == type; }

	T_Value m_data;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	NamedProperties() = default;
	NamedProperties(NamedProperties const& otherNamedProperties);
	~NamedProperties();

	template<typename T_Value>
	inline T_Value GetValue(std::string const& name, T_Value const& defaultValue) const;
	
	template<typename T_Value>
	inline void SetValue(std::string const& name, T_Value const& value);

	inline std::string GetValue(std::string const& name, char const* defaultValue) const;
	inline void SetValue(std::string const& name, char const* value);
	
	void PopulateFromXmlElementAttributes(XmlElement const& element);

	void operator=(NamedProperties const& otherNamedProperties);

	std::map<HashedCaseInsensitiveString, NamedPropertyBase*> m_keyValuePairs;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_Value>
inline T_Value NamedProperties::GetValue(std::string const& name, T_Value const& defaultValue) const
{
	auto it = m_keyValuePairs.find(HashedCaseInsensitiveString(name));
	if (it != m_keyValuePairs.end())
	{
		NamedPropertyBase const* anyTypeProperty = it->second;
		if (anyTypeProperty->IsType(typeid(defaultValue))) 
		{
			NamedProperty<T_Value> const* propertyAsCorrectType = static_cast<NamedProperty<T_Value> const*>(anyTypeProperty);
			return propertyAsCorrectType->m_data;
		}
		else 
		{
			ERROR_AND_DIE("Type mismatch in GetValue for NamedProperties.");
		}
	}
	return defaultValue;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_Value>
inline void NamedProperties::SetValue(std::string const& name, T_Value const& value)
{
	HashedCaseInsensitiveString hashedKey(name);
	auto it = m_keyValuePairs.find(hashedKey);
	if (it != m_keyValuePairs.end()) 
	{
		NamedPropertyBase* anyTypeProperty = it->second;
		if (anyTypeProperty->IsType(typeid(value))) 
		{
			NamedProperty<T_Value>* propertyAsCorrectType = static_cast<NamedProperty<T_Value>*>(anyTypeProperty);
			propertyAsCorrectType->m_data = value;
		}
		else 
		{
			delete anyTypeProperty; 
			m_keyValuePairs.erase(it); 
			ERROR_AND_DIE("Type mismatch in SetValue for NamedProperties. Old value type does not match new value type.");
		}
	}
	else
	{
		NamedProperty<T_Value>* newProperty = new NamedProperty<T_Value>();
		newProperty->m_data = value;
		m_keyValuePairs[hashedKey] = newProperty;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
inline std::string NamedProperties::GetValue(std::string const& name, char const* defaultValue) const
{
	return GetValue<std::string>(name, std::string(defaultValue));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
inline void NamedProperties::SetValue(std::string const& name, char const* value)
{
	SetValue<std::string>(name, value);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------