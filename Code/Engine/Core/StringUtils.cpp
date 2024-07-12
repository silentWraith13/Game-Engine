#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn)
{
	Strings returnValue;
	size_t pos = 0;
	size_t lastPos = 0;
	while ((pos = originalString.find(delimiterToSplitOn, lastPos)) != std::string::npos) {
		returnValue.push_back(originalString.substr(lastPos, pos - lastPos)); // Get substring from lastPos to the character before the delimiter
		lastPos = pos + 1; // Move past the delimiter for the next search
	}
	returnValue.push_back(originalString.substr(lastPos)); // Add the last part of the string after the final delimiter
	return returnValue;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiterExcludeConsecutives(std::string const& originalString, char delimiterToSplitOn)
{
	Strings returnValue;
	size_t pos = 0;
	size_t prevPos = 0;

	while ((pos = originalString.find(delimiterToSplitOn, prevPos)) != std::string::npos) {
		if (pos > prevPos) {  // Exclude consecutive delimiters
			returnValue.push_back(originalString.substr(prevPos, pos - prevPos));
		}
		prevPos = pos + 1; // Move past the delimiter
	}

	// Push the last token if it's not empty
	if (prevPos < originalString.length()) {
		returnValue.push_back(originalString.substr(prevPos));
	}

	return returnValue;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TrimString(std::string& originalString, char delimiterToTrim)
{
	while (!originalString.empty() && originalString.front() == delimiterToTrim) 
	{
		originalString.erase(originalString.begin());
	}

	while (!originalString.empty() && originalString.back() == delimiterToTrim) 
	{
		originalString.pop_back();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Strings SplitStringWithDelimeterWithQuotes(const std::string& originalString, char delimiterToSplitOn)
{
	Strings result;
	size_t delimeterPos = 0;
	bool isInQuotes = false;
	for (int i = 0 ;i < originalString.size(); i++)
	{
		if (originalString[i] == '"')
		{
			isInQuotes = !isInQuotes;
		}

		if (!isInQuotes && originalString[i] == delimiterToSplitOn)
		{
			
			std::string temp = std::string(originalString, delimeterPos, i - delimeterPos);
			result.push_back(temp);
			delimeterPos = i + 1;

		}
	}

	std::string subString = std::string(originalString, delimeterPos);
	result.push_back(subString);
	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void StringToEventArgs(const std::string& input, EventArgs& outArgs)
{
	Strings keyValuePairs = SplitStringOnDelimiter(input, ' ');

	for (const std::string& pair : keyValuePairs)
	{
		Strings splitPair = SplitStringOnDelimiter(pair, '=');
		if (splitPair.size() == 2)
		{
			std::string key = splitPair[0];
			std::string value = splitPair[1];

			outArgs.SetValue(key, value);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::string ExtractInnerCommand(const std::string& commandContent)
{
	size_t startQuote = commandContent.find('"');
	if (startQuote == std::string::npos) {
		return ""; // No starting quote found
	}
	size_t endQuote = commandContent.find('"', startQuote + 1);
	if (endQuote == std::string::npos) {
		return ""; // No ending quote found
	}

	return commandContent.substr(startQuote + 1, endQuote - startQuote - 1);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AreStringsEqualCaseInsensitive(std::string const& stringA, std::string const& stringB)
{
	return !_stricmp(stringA.c_str(), stringB.c_str());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AreStringsEqualCaseInsensitive(const HashedCaseInsensitiveString& stringA, const HashedCaseInsensitiveString& stringB)
{
	return stringA == stringB;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AreStringsEqualCaseInsensitive(const std::string& stringA, const HashedCaseInsensitiveString& stringB)
{
	return _stricmp(stringA.c_str(), stringB.GetString().c_str()) == 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool AreStringsEqualCaseInsensitive(const HashedCaseInsensitiveString& stringA, const std::string& stringB)
{
	return _stricmp(stringA.GetString().c_str(), stringB.c_str()) == 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

