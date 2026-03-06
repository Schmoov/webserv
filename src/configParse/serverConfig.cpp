#include "../../inc/configParse/configParse.hpp"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

void configParseServerName(ServerConfig& conf, std::string& line) {
    std::stringstream ss(line);
    std::string token;
    ss >> token;
    if (!(ss >> conf.server_name) || ss >> token)
        configErrorLog("server_name <name>", line);
}

void configParseHost(ServerConfig& conf, std::string& line) {
    std::stringstream ss(line);
    std::string token;
    ss >> token; // host
    if (!(ss >> conf.host) || ss >> token)
        configErrorLog("host <IP>", line);
}

void configParseRoot(ServerConfig& conf, std::string& line) {
    std::stringstream ss(line);
    std::string token;
    ss >> token; // root
    if (!(ss >> conf.root) || ss >> token)
        configErrorLog("root <path>", line);
}

void configParseBodySize(ServerConfig& conf, std::string& line) {
    std::stringstream ss(line);
    std::string token;
    ss >> token; // client_max_body_size
    if (!(ss >> token) || ss >> token)
        configErrorLog("client_max_body_size <size>", line);

    size_t size;
    try {
		size = extractSize(token);
	} catch (std::exception& e) {
		configErrorLog("client_max_body_size <size>", line);
	}

    conf.clientMaxBodySize = size;
}

//doesnt check statusCode validity
void configParseErrorPage(ServerConfig& conf, std::string& line) {
    std::stringstream ss(line);
    std::string token;
    int code;
    std::string path;

    ss >> token; // error_page
    if (!(ss >> code) || !(ss >> path) || (ss >> token))
		configErrorLog("error_page <code> <path>", line);

    conf.errorPagesPath[(StatusCode)code] = path;
}
