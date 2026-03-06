#pragma once

#include <string>

void configThrow(std::string what);
void configErrorLog(std::string expected, std::string& actual);
std::string parseNextLine(std::ifstream& file);
