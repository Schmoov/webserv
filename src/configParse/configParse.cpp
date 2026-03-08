#include "../../inc/configParse/configParse.hpp"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

map<int, ServerConfig> parseConfig(string path) {
	ifstream file(path.c_str());
	if (!file) {
		cerr << "Cannot open: " << path << endl;
		configThrow("Bad path");
	}

	map<int, ServerConfig> portToConf;
	while (parseServer(portToConf, file)) {
	}
	return portToConf;
}

bool parseServer(map<int, ServerConfig>& portToConf, ifstream& file) {
	string line = parseNextLine(file);
	if (line == "")
		return false;

	if (line != "server {")
		configErrorLog("server {", line);

	int port = parseServerPort(file);
	portToConf[port].port = port;

	parseServerParams(portToConf[port], file);
	return true;
}

int parseServerPort(ifstream& file) {
	string line = parseNextLine(file);
	stringstream iss(line);
	string token;
	if (!(iss >> token) || token != "listen")
		configErrorLog("listen", line);

	if (!(iss >> token))
		configErrorLog("listen [PORT]", line);

	int res;
	try {
		res = extractSize(token);
	} catch (exception& e) {
		configErrorLog("listen [PORT]", line);
	}

	if (iss >> token)
		configErrorLog("listen [PORT]", line);

	return res;
}

void parseServerParams(ServerConfig& conf, ifstream& file) {
	string line = parseNextLine(file);

	while (line != "}") {
		if (line == "") {
			cerr << "server block not closed" << endl;
			configThrow("missing server }");
		}

		string token = line.substr(0, line.find(' '));

		if (token == "location")
			parseLocation(conf, line, file);
		else if (token == "server_name")
			configParseServerName(conf, line);
		else if (token == "host")
			configParseHost(conf, line);
		else if (token == "root")
			configParseRoot(conf, line);
		else if (token == "client_max_body_size")
			configParseBodySize(conf, line);
		else if (token == "error_page")
			configParseErrorPage(conf, line);
		else {
			cerr << "Unknown directive in server block: " << token << endl;
			configThrow("Invalid config directive");
		}

		line = parseNextLine(file);
	}
}

void parseLocation(ServerConfig& conf, string& line, ifstream& file) {
	stringstream ss(line);
	string token;
	string name;

	ss >> token; // location
	ss >> name;

	if (name == "")
		configErrorLog("location /path {", line);

	if (!(ss >> token) || token != "{")
		configErrorLog("location /path {", line);

	Location loc;
	loc.name = name;

	line = parseNextLine(file);
	while (line != "}") {

		if (line == "") {
			cerr << "location block not closed" << endl;
			configThrow("missing location }");
		}

		string key = line.substr(0, line.find(' '));

		if (key == "allow_methods")
			configParseAllowMethods(loc, line);
		else if (key == "autoindex")
			configParseAutoIndex(loc, line);
		else if (key == "index")
			configParseIndex(loc, line);
		else if (key == "root")
			configParseRoot(loc, line);
		else if (key == "return")
			configParseReturn(loc, line);
		else if (key == "cgi_path")
			configParseCgiPath(loc, line);
		else if (key == "cgi_ext")
			configParseCgiExt(loc, line);
		else {
			cerr << "Unknown directive in location: " << key << endl;
			configThrow("Invalid location directive");
		}
		line = parseNextLine(file);
	}

	configParseCheckCGI(loc);
	conf.pathToLoc[name] = loc;
}

void configParseCheckCGI(Location& loc) {
    if (loc.CgiExt.size() != loc.CgiPath.size()) {
        cerr << "Must define 1 (one) CGI ext per binary\n";
        configThrow("CGI Mismatch");
    }
    for (int i = 0; i < (int)loc.CgiExt.size(); i++)
        loc.cgiHandler[loc.CgiExt[i]] = loc.CgiPath[i];
}
