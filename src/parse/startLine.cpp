#include "../../inc/parse/parse.hpp"

using namespace std;
using namespace ParserRoutine;

namespace ParserRoutine {

string extractMethod(string& s) {
	string res = extractToken(s);
	if (res.empty() || s[0] != ' ')
		parseThrow("Bad method");
	s.erase(0, 1);
	return res;
}

// "authority is a valid URI so it shouldnt err, but it is hard to parse and
// useless because it is only valid for CONNECT that we will not implement..."
// Thus i would like to delay the linting of the URI to the validator
static bool isValidUri(const string& s) {return s.size();}

string extractRequestUri(string& s) {
	size_t pos = s.find(' ');
	if (pos == npos)
		parseThrow("Bad URI");
	string res = s.substr(0, pos);
	if (!isValidUri(res))
		parseThrow("Bad URI");
	s.erase(0, pos+1);
	return res;
}

pair<int, int> extractHttpVersion(string& s) {
	if (s.compare(0, 5, "HTTP/"))
		parseThrow("Bad version");
	s.erase(0, 5);
	pair<int, int> res;
	res.first = extractSize(s);
	if (s[0] != '.')
		parseThrow("Bad version");
	s.erase(0, 1);
	res.second = extractSize(s);
	if (s.compare(0, 2, "\r\n"))
		parseThrow("Bad start line");
	s.erase(0, 2);
	return res;
}

}
