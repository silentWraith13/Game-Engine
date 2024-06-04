#include "NamedProperties.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
HashedCaseInsensitiveString::HashedCaseInsensitiveString(std::string const& value)
{
	m_textCaseIntact = value;
	m_lowerCaseHash = CalculateHash(value);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int HashedCaseInsensitiveString::CalculateHash(std::string const& text) const
{
	unsigned int hash = 0; 
	for (char letter : text)
	{
		hash *= 31;
		hash += static_cast<unsigned int>(std::tolower(static_cast<unsigned char>(letter))); 
	}
	return hash;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator<(const HashedCaseInsensitiveString& other) const
{
	return m_lowerCaseHash < other.m_lowerCaseHash;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator==(HashedCaseInsensitiveString const& compare) const
{
	if (m_lowerCaseHash == compare.m_lowerCaseHash)
	{
		if (_stricmp(m_textCaseIntact.c_str(), compare.m_textCaseIntact.c_str()) == 0)
		{
			return true; 
		}
		else
		{
			return false; 
		}
	}
	else
	{
		return false; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator!=(HashedCaseInsensitiveString const& compare) const
{
	if (*this == compare)
	{
		return false;
	}
	else
	{
		return true; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator==(std::string const& compareString) const
{
	if (m_lowerCaseHash == CalculateHash(compareString))
	{
		if (_stricmp(m_textCaseIntact.c_str(), compareString.c_str()) == 0)
		{
			return true; 
		}
		else
		{
			return false; 
		}
	}
	else
	{
		return false; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator!=(std::string const& compareString) const
{
	if (*this == compareString)
	{
		return false;
	}
	else
	{
		return true;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
NamedProperties::NamedProperties(NamedProperties const& otherNamedProperties)
{
	std::map<HashedCaseInsensitiveString, NamedPropertyBase*> const& otherProperties = otherNamedProperties.m_keyValuePairs;

	std::map<HashedCaseInsensitiveString, NamedPropertyBase*>::const_iterator it;
	for (it = otherProperties.begin(); (it != otherProperties.end()); it++)
	{
		m_keyValuePairs[it->first] = it->second->GetClone();

	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
NamedProperties::~NamedProperties()
{
	for (auto& [key, value] : m_keyValuePairs) 
	{
		if (value) 
		{
			delete value;
			value = nullptr;
		}
	}
	m_keyValuePairs.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NamedProperties::operator=(NamedProperties const& otherNamedProperties)
{
	for (auto& [key, value] : m_keyValuePairs) 
	{
		if (value)
		{
			delete value; 
		}
		else
		{
			ERROR_AND_DIE("Unexpected nullptr in NamedProperties assignment operator.");
		}
	}
	m_keyValuePairs.clear();

	for (auto& [key, otherProp] : otherNamedProperties.m_keyValuePairs)
	{
		m_keyValuePairs[key] = otherProp->GetClone();
		if (!m_keyValuePairs[key])
		{
			ERROR_AND_DIE("Failed to clone property in NamedProperties assignment operator.");
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NamedProperties::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attrib = element.FirstAttribute();
	while (attrib)
	{
		std::string attrName = attrib->Name();
		std::string attrValue = attrib->Value();
		SetValue<std::string>(attrName, attrValue);
		attrib = attrib->Next();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------