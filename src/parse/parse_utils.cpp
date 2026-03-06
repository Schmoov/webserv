#include <iostream>
#include <sstream>
#include <exception>
#include <string>
#include "../../inc/parse/parse_utils.hpp"

using namespace std;

void parseThrow(string what) {
#ifdef PARSE_DEBUG
	cerr << what << endl;
#endif
	(void) what;
	throw exception();
}

string extractToken(string& s) {
	size_t pos = s.find_first_not_of(tchar);
	if (pos == npos)
		pos = s.size();
	string res = s.substr(0, pos);
	s.erase(0, pos);
	return res;
}

void deleteLeadOWS(string& s) {
	size_t pos = s.find_first_not_of(" \t");
	if (pos == npos)
		pos = s.size();
	s.erase(0, pos);
}

void deleteTrailOWS(string& s) {
	size_t pos = s.find_last_not_of(" \t");
	if (pos == npos)
		return ;
	s.erase(pos + 1, s.size() - pos - 1);
}

void deleteChunkExt(string& s) {
	while (s.compare(0, 2, "\r\n")) {
		deleteLeadOWS(s);
		if (s[0] == ';') {
			s.erase(0, 1);
			deleteLeadOWS(s);
			string tok = extractToken(s);
			if (tok == "")
				parseThrow("Bad chunk extension");
			deleteLeadOWS(s);
			if (s[0] == '=') {
				s.erase(0, 1);
				deleteLeadOWS(s);
				if (s[0] == '"')
					deleteQuotedString(s);
				else {
					tok = extractToken(s);
					if (tok == "")
						parseThrow("Bad chunk extension");
				}
			}
		}
	}
}

void deleteQuotedString(string& s) {
	size_t i = 0;

	if (s[i] != '"')
		parseThrow("Bad quoted string");
	i++;
	while (i < s.size() && s[i] != '"') {
		if ((s[i] >= 0 && s[i] < ' ' && s[i] != '\t') || s[i] == 127)
			parseThrow("Bad quoted string");
		if (s[i] == '\\') {
			i++;
			if ((s[i] >= 0 && s[i] < ' ' && s[i] != '\t') || s[i] == 127)
			parseThrow("Bad quoted string");
		}
			i++;
	}
	if (s[i] != '"')
		parseThrow("Bad quoted string");
	s.erase(0, i+1);
}

size_t peekSize(string& s, int base) {
	size_t pos;
	if (base == 16)
		pos = s.find_first_not_of(base16);
	else
		pos = s.find_first_not_of(base10);
	if (pos == npos)
		pos = s.size();
	stringstream ss(s.substr(0,pos));
	size_t res;
	if (base == 16)
		ss >> std::hex;
	if (!(ss>>res)) {
		if (pos)
			throw std::overflow_error("Bad size_t");
		else
			throw std::invalid_argument("Bad size_t");
	}
	return res;
}

//Calling with 2 params sets close to true
void earlyResponse(Conversation& conv, StatusCode status, bool close) {
	conv.state = EXEC;
	conv.resp.status = status;
	conv.resp.shouldClose = close;
}
