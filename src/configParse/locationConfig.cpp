#include "../../inc/configParse/configParse.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

void configParseAllowMethods(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // allow_methods

    loc.allowedMethods.clear();
    while (ss >> token) {
        loc.allowedMethods.push_back(token);
    }

    if (loc.allowedMethods.empty()) {
        configErrorLog("allow_methods <METHODS...>", line);
    }
}

void configParseAutoIndex(Location& loc, string& line) {
    stringstream ss(line);
    string isOn;
    string token;
    ss >> token; // autoindex

    if (!(ss >> isOn) || (isOn != "on" && isOn != "off") || (ss >> token)) {
        configErrorLog("autoindex on|off", line);
    }

    loc.autoIndex = (isOn == "on");
}

void configParseIndex(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // index

    if (!(ss >> loc.index) || (ss >> token)) {
        configErrorLog("index <file>", line);
    }
}

void configParseRoot(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // root

    if (!(ss >> loc.root) || (ss >> token)) {
        configErrorLog("root <path>", line);
    }
    loc.hasRoot = true;
}

void configParseReturn(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    int code;

    ss >> token; // return
    if (!(ss >> token)) {
        configErrorLog("return <code> <url>", line);
    }

    try {
        stringstream conv(token);
        conv >> code;
    } catch (...) {
        configErrorLog("return <code> <url>", line);
    }
    loc.redirCode = (StatusCode)code;

    if (!(ss >> loc.redirURL) || (ss >> token)) {
        configErrorLog("return <code> <url>", line);
    }

    loc.hasRedir = true;
}

void configParseCgiPath(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // cgi_path
    while (ss >> token) {
        loc.CgiPath.push_back(token);
    }
}

void configParseCgiExt(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // cgi_ext
    while (ss >> token) {
        loc.CgiExt.push_back(token);
    }
}
