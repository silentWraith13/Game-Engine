#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DoesFileExist(const std::string& filename);
int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename);
int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
int FileReadToString(std::string& outString, const std::string& filename);
int FileSize(FILE* fileStream);
//--------------------------------------------------------------------------------------------------------------------------------------------------------