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
	std::map<std::string, std::string> cgiHandlers;
};

struct ServerConfig {
	unsigned int	port;
	std::string		host;
	std::string		server_name;
	std::map<StatusCode, std::string> errorPagesPath;
	std::map<std::string, Location> pathToLoc;
};
