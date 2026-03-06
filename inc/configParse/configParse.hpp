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
void configParseAllowMethods(Location& loc, std::string& line);
void configParseAutoIndex(Location& loc, std::string& line);
void configParseIndex(Location& loc, std::string& line);
void configParseRoot(Location& loc, std::string& line);
void configParseReturn(Location& loc, std::string& line);
void configParseCgiPath(Location& loc, std::string& line);
void configParseCgiExt(Location& loc, std::string& line);
