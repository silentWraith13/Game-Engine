#pragma once
#include "Engine/Core/EventSytem.hpp"
#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NamedStrings;
class EventSystem;
class HashedCaseInsensitiveString;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
typedef std::vector< std::string >	Strings;
//-----------------------------------------------------------------------------------------------
const std::string	Stringf( char const* format, ... );
const std::string	Stringf( int maxLength, char const* format, ... );
Strings				SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn);
Strings				SplitStringOnDelimiterExcludeConsecutives(std::string const& originalString, char delimiterToSplitOn);
void				TrimString(std::string& originalString, char delimiterToTrim);
Strings				SplitStringWithDelimeterWithQuotes(const std::string& originalString, char delimiterToSplitOn);
void				StringToEventArgs(const std::string& input, EventArgs& outArgs);
std::string			ExtractInnerCommand(const std::string& commandContent);
bool				AreStringsEqualCaseInsensitive(std::string const& stringA, std::string const& stringB);
bool				AreStringsEqualCaseInsensitive(const HashedCaseInsensitiveString& stringA, const HashedCaseInsensitiveString& stringB);
bool				AreStringsEqualCaseInsensitive(const std::string& stringA, const HashedCaseInsensitiveString& stringB);
bool				AreStringsEqualCaseInsensitive(const HashedCaseInsensitiveString& stringA, const std::string& stringB);
//--------------------------------------------------------------------------------------------------------------------------------------------------------


