#pragma once

#include "parse.hpp"

class Parser {
public:
	ParseState pState; // public for tests...
private:

	void parseStartLine(Conversation& conv);
	void handleHugeStart(Conversation& conv);
	void parseHeader(Conversation& conv);
	void parseBody(Conversation& conv);
	void parseBodyChunked(Conversation& conv);
	void parseTrailer(Conversation& conv);

public:
	void parse(Conversation& conv);
	Parser();
};
