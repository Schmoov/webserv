#pragma once

#include "../webserv.hpp"
#include "configParseUtils.hpp"
#include <fstream>

//throws if parse fails
std::map<int, ServerConfig> parseConfig(std::string path);

bool parseServer(std::map<int, ServerConfig>& portToConf, std::ifstream& file);
void parseServerParams(ServerConfig& conf, std::ifstream& file);

int parseServerPort(std::ifstream& file);
void configParseServerName(ServerConfig& conf, std::string& line);
void configParseHost(ServerConfig& conf, std::string& line);
void configParseRoot(ServerConfig& conf, std::string& line);
void configParseBodySize(ServerConfig& conf, std::string& line);
void configParseErrorPage(ServerConfig& conf, std::string& line);
