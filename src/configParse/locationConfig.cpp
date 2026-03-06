#pragma once

#include "../../inc/configParse/configParse.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

// Parses "allow_methods GET POST ..." into loc.allowedMethods
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

// Parses "autoindex on|off" into loc.autoIndex
void configParseAutoIndex(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // autoindex

    if (!(ss >> token) || (token != "on" && token != "off") || (ss >> token)) {
        configErrorLog("autoindex on|off", line);
    }

    loc.autoIndex = (token == "on");
}

// Parses "index index.html ..." into loc.index (takes first token)
void configParseIndex(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // index

    if (!(ss >> token) || (ss >> token)) { // only one index allowed
        configErrorLog("index <file>", line);
    }

    loc.index = token;
}

// Parses "root /some/path" into loc.root
void configParseRoot(Location& loc, string& line) {
    stringstream ss(line);
    string token;
    ss >> token; // root

    if (!(ss >> token) || (ss >> token)) {
        configErrorLog("root <path>", line);
    }

    loc.root = token;
    loc.hasRoot = true;
}

// Parses "return 301 /new/path" into loc.redirCode and loc.redirURL
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

    if (!(ss >> token) || (ss >> token)) {
        configErrorLog("return <code> <url>", line);
    }

    loc.hasRedir = true;
    loc.redirCode = (StatusCode)code;
    loc.redirURL = token;
}

// Parses "cgi_path /usr/bin/php" into loc.cgiHandlers["default"] = path
void configParseCgiPath(Location& loc, string& line) {
    stringstream ss(line);
    string token, path;
    ss >> token; // cgi_path
    if (!(ss >> path) || (ss >> token)) {
        configErrorLog("cgi_path <path>", line);
    }

    loc.cgiHandlers["default"] = path;
}

// Parses "cgi_ext .php" into loc.cgiHandlers["ext"] = ext
void configParseCgiExt(Location& loc, string& line) {
    stringstream ss(line);
    string token, ext;
    ss >> token; // cgi_ext
    if (!(ss >> ext) || (ss >> token)) {
        configErrorLog("cgi_ext <ext>", line);
    }

    // associate extension with existing default CGI path
    if (loc.cgiHandlers.find("default") == loc.cgiHandlers.end()) {
        configErrorLog("cgi_ext defined before cgi_path", line);
    }

    loc.cgiHandlers[ext] = loc.cgiHandlers["default"];
}
