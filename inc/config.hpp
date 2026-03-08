#pragma once

#include "enum.hpp"
#include <string>
#include <map>
#include <vector>

struct Location {

	std::string name;
	bool hasRoot;
	std::string root;
	std::vector<std::string> allowedMethods;
	std::string index;
	bool autoIndex;
	bool hasRedir;
	StatusCode redirCode;
	std::string redirURL;
	std::string uploadDir;
	std::map<std::string, std::string> cgiHandler; //"py"-->"/bin/python3"

	//private
	std::vector<std::string> CgiExt;
	std::vector<std::string> CgiPath;

	Location() : hasRoot(false), autoIndex(false), hasRedir(false),
		redirCode(NOT_A_STATUS_CODE) {}

};

struct ServerConfig {
	unsigned int	port;
	std::string root;
	std::string		host;
	std::string		server_name;
	size_t clientMaxBodySize;
	std::map<StatusCode, std::string> errorPagesPath;
	std::map<std::string, Location> pathToLoc;
	ServerConfig() : port(0), clientMaxBodySize(1e6) {}
};
