#pragma once
#include <string>
#include <map>
#include <stdint.h>

typedef std::map<std::string, std::string> mapStr;
typedef std::pair<std::string, std::string> pairStr;

const size_t npos = std::string::npos;
const std::string base10 = "0123456789";
const std::string base16 = base10 + "abcdefABCDEF";
const std::string loalpha = "abcdefghijklmnopqrstuvwxyz";
const std::string upalpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string alpha = loalpha + upalpha;

std::string intToString(int n);
void toLower(std::string& s, size_t start = 0, size_t len = SIZE_MAX);
//Throws "invalid_argument" (if s doesnt start with a proper size_t) or "overflow_error" !
size_t extractSize(std::string& s, int base=10);
