#include "../inc/utils.hpp"
#include <sstream>

std::string intToString(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

void toLower(std::string& s, size_t start, size_t len) {
	for (size_t i = start; i < s.size() && i-start < len; i++)
		s[i] = tolower(s[i]);
}

//Calling with 1 param sets base to 10
size_t extractSize(std::string& s, int base) {
	size_t pos;
	if (base == 16)
		pos = s.find_first_not_of(base16);
	else
		pos = s.find_first_not_of(base10);
	if (pos == npos)
		pos = s.size();
	std::stringstream ss(s.substr(0,pos));
	s.erase(0, pos);
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
