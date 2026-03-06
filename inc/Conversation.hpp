#pragma once

#include <string>
#include <map>
#include "enum.hpp"
#include "utils.hpp"
#include "config.hpp"

class Request {
public:
	std::string method;
	std::pair<int, int> version;
	std::string uri; // Could be vector (a/b vs [a, b])
	mapStr header;
	size_t bodyLeft;
	std::string body;
	std::string pathOnDisk;
	bool hasQuery;
	std::string query;
};

struct Response {
	StatusCode status; // Used by parse to communicate failure
	bool shouldClose;
	std::string location;
	/*
	mapStr header;
	std::string body;
	*/
};

//Forward declaration + pointer to respect include hierarchy
class Parser;  
class Reader;
class Validator;

class Conversation {
private:
	Parser *parser;
	Reader *reader;
	Validator *validator;
public:
	int fd;
	ServerConfig *conf;
	Location *loc;
	Request req;
	Response resp;
	ConvState state;
	std::string buf;
};
