#pragma once
#include "../webserv.hpp"

class Reader {
private:
	static const int BUFFER_SIZE = (1LL << 16);
public:
	void read(Conversation& conv);
};

