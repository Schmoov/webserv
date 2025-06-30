#include <string>
#include <sstream>

class 
class Request {
private:
public:
	std::stringstream raw;
	std::string startLine;
	std::string headers;
	std::string body;
};

class Conversation {
private:
public:
	int fd;
	std::string domain;
	Request req;
	Response resp;
};


