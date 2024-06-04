#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <filesystem>
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DoesFileExist(const std::string& filename)
{
	struct stat buffer;
	int status = stat(filename.c_str(), &buffer);
	return status == 0 ? true : false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename)
{
	std::filesystem::path filePath(filename);

	std::filesystem::path directoryPath = filePath.remove_filename();
	if (!std::filesystem::exists(directoryPath)) 
	{
		std::filesystem::create_directory(directoryPath);
	}

	std::ofstream outfile(filename, std::ios::binary | std::ios::out);
	char const* bufferToChar = reinterpret_cast<char*>(inBuffer.data());
	outfile.write(bufferToChar, inBuffer.size() * sizeof(uint8_t));

	if (outfile.bad()) 
	{
		ERROR_RECOVERABLE(Stringf("ERROR TRYING TO WRITE FROM BUFFER TO FILE: %s", filename.c_str()));
		return -1;
	}

	outfile.close();
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& fileName)
{
	std::ifstream inFile(fileName, std::ifstream::in | std::ifstream::binary);
	size_t fileSize = static_cast<size_t>(inFile.tellg());

	inFile.seekg(0, std::ios::end);

	fileSize = static_cast<size_t>(inFile.tellg()) - fileSize;
	outBuffer.resize(fileSize);

	inFile.seekg(0, std::ios::beg);

	if (!inFile.is_open()) 
	{
		inFile.close();
		ERROR_RECOVERABLE(Stringf("ERROR TRYING TO OPEN THE FILE, MAY NOT EXIST %s", fileName.c_str()));
	}
	else {
		inFile.read(reinterpret_cast<char*>(outBuffer.data()), fileSize);
	}

	if (inFile.bad())
	{
		inFile.close();
		ERROR_RECOVERABLE(Stringf("ERROR TRYING TO READ TO  BUFFER TO FILE: %s", fileName.c_str()));
		return -1;
	}

	inFile.close();
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int FileReadToString(std::string& outString, const std::string& fileName)
{
	std::vector<uint8_t> fileBuffer;
	int result = FileReadToBuffer(fileBuffer, fileName);
	if (result != 0)
	{
		return -1;
	}

	fileBuffer.push_back('\0');

	outString = std::string((char*)fileBuffer.data());

	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int FileSize(FILE* fileStream)
{
	int prev = ftell(fileStream);
	fseek(fileStream, 0L, SEEK_END);
	int sz = ftell(fileStream);
	fseek(fileStream, prev, SEEK_SET);
	return sz;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------