#include "../../inc/serverConfig.hpp"
#include "../../inc/parse/parse_utils.hpp"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

void configThrow(string what) {
#ifdef CONFIG_DEBUG
	cerr << what << endl;
#endif
	(void) what;
	throw exception();
}

void configErrorLog(string expected, string& actual) {
	cerr << "Error: expected \"" << expected
			<< "\" found \"" << actual << "\" \n";
	configThrow(expected);
}

//skips empty lines, collapses WS, returns "" on eof
string parseNextLine(ifstream& file) {
	string line;
	while (getline(file, line)) {
		line = line.substr(0, line.find('#'));
		istringstream iss(line);
		string res;
		if (!(iss >> res))
			continue;

		string token;
		while (iss >> token) {
			res += ' ';
			res += token;
		}
		return res;
	}
	return "";
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

void parseServerParams(map<int, ServerConfig>& portToConf, ifstream& file) {
	int port = parseServerPort(file);
	portToConf[port].port = port;

	string line = parseNextLine(file);
	while (line != "}") {
		if (line == "") {
			cerr << "server '{' was never closed" << endl;
			configThrow("missing server }");
		}
		std::string token = line.substr(0, line.find(' '));
		if (token == location) {
			configParseLocation(line);
		} if (token == index) {
			configParseLocation(line);
		}
			
}


bool parseServer(map<int, ServerConfig>& portToConf, ifstream& file) {
	string line = parseNextLine(file);
	if (line == "")
		return false;

	if (line != "server {")
		configErrorLog("server {", line);

	parseServerParams(file);
}

//throws if parse fails
map<int, ServerConfig> parseConfig(string path) {
	ifstream file(path);
	if (!file) {
		cerr << "Cannot open: " << path << endl;
		configThrow("Bad path");
	}

	map<int, ServerConfig> portToConf;
	while (parseServer(portToConf, file)) {
	}
	return portToConf;
}
