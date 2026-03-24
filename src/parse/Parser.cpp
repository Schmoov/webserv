#include "../../inc/parse/Parser.hpp"
#include <exception>
#include <stdexcept>
#include <iostream>

using namespace std;
using namespace ParserRoutine;

Parser::Parser() : pState(START) {}

void Parser::parseStartLine(Conversation& conv) {
	string& s = conv.buf;
	while (!s.compare(0, 2, "\r\n"))
		s.erase(0, 2);
	size_t pos = s.find("\r\n");
	if (pos == npos && s.size() <= startLineMax)
		return;
	if ((pos != npos && pos > startLineMax)
			|| pos == npos) {
		handleHugeStart(conv);
		return;
	}
	conv.req.method = extractMethod(s);
	conv.req.uri = extractRequestUri(s);
	conv.req.version = extractHttpVersion(s);
	pState = HEADER;
}

void Parser::handleHugeStart(Conversation& conv) {
	std::string res = extractMethod(conv.buf);
	if (res.size() > methodMax)
		return earlyResponse(conv, NOT_IMPLEMENTED);
	res = extractRequestUri(conv.buf);
	if (res.size() > uriMax)
		return earlyResponse(conv, URI_TOO_LONG);
	parseThrow("Bad start line");
}

void Parser::parseHeader(Conversation& conv) {
	string& s = conv.buf;
	size_t pos = s.find("\r\n\r\n");
	if (pos == npos) {
		if (s.size() > headerMax)
			return earlyResponse(conv, REQUEST_HEADER_FIELDS_TOO_LARGE);
		conv.state = READ_CLIENT;
		return;
	}
	conv.req.header = parseAllField(s);
	conv.req.body = "";
	conv.state = VALIDATE;
	pState = START;
	return;
}

void Parser::parseBody(Conversation& conv) {
	if (conv.req.header.count("content-length")) {
		size_t pos = min(conv.buf.size(), conv.req.bodyLeft);
		conv.req.body += conv.buf.substr(0, pos);
		conv.req.bodyLeft -= pos;
		conv.buf.erase(0, pos);
		if (conv.req.bodyLeft) {
			conv.state = READ_CLIENT;
			return;
		}
		conv.state = EXEC;
		pState = START;
	} else {
		parseBodyChunked(conv);
	};
}

void Parser::parseBodyChunked(Conversation& conv) {
	string& s = conv.buf;
	while (true) {
		size_t pos = s.find("\r\n");
		if (pos == npos && s.size() <= bodyMax) {
			conv.state = READ_CLIENT;
			return;
		}
		if ((pos != npos && pos > bodyMax) || pos == npos)
			return earlyResponse(conv, BAD_REQUEST);

		size_t chunkSize;
		try {
			chunkSize = peekSize(s, 16);
		} catch (std::overflow_error& e) {
			return earlyResponse(conv, ENTITY_TOO_LARGE);
		}

		if (s.size() - (pos + 2) < chunkSize + 2)
			return;
		extractSize(s, 16);
		deleteChunkExt(s);

		if (!chunkSize) {
			pState = TRAILER;
			return;
		}
		s.erase(0, 2);
		if (conv.req.body.size() + chunkSize > bodyMax)
			return earlyResponse(conv, ENTITY_TOO_LARGE);
		conv.req.body += s.substr(0, chunkSize);
		s.erase(0, chunkSize);
		if (s.compare(0, 2, "\r\n"))
			parseThrow("Bad chunk");
		s.erase(0, 2);
	}
}

void Parser::parseTrailer(Conversation& conv) {
	string& s = conv.buf;
	size_t pos = s.find("\r\n\r\n");
	if (pos == npos) {
		if (s.size() > headerMax)
			return earlyResponse(conv, REQUEST_HEADER_FIELDS_TOO_LARGE);
		conv.state = READ_CLIENT;
		return;
	}
	parseAllField(s);
	conv.state = EXEC;
	pState = START;
	return;
}

void Parser::parse(Conversation& conv) {
	try {
		if (conv.state == PARSE_BODY)
			pState = BODY;
		if (conv.state == EOF_CLIENT) {
			if (conv.buf.empty() && pState == START)
				conv.state = FINISH;
			else {
				return earlyResponse(conv, BAD_REQUEST);
			}
			return;
		}

		if (pState == START)
			parseStartLine(conv);
		if (pState == HEADER)
			parseHeader(conv);
		if (pState == BODY)
			parseBody(conv);
		if (pState == TRAILER)
			parseTrailer(conv);
	} catch (std::exception& e) {
		return earlyResponse(conv, BAD_REQUEST);
	}
}
