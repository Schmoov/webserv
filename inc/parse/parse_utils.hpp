#pragma once
#include <map>
#include <string>

#include "../webserv.hpp"

const size_t npos = std::string::npos;
const std::string base10 = "0123456789";
const std::string base16 = base10 + "abcdefABCDEF";
const std::string loalpha = "abcdefghijklmnopqrstuvwxyz";
const std::string upalpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string alpha = loalpha + upalpha;
const std::string tchar = alpha + "!#$%&'*+-.^_|~";

std::string intToString(int n);
void toLower(std::string& s, size_t start = 0, size_t len = SIZE_MAX);
//Throws "invalid_argument" (if s doesnt start with a proper size_t) or "overflow_error" !
size_t extractSize(std::string& s, int base=10);
size_t peekSize(std::string& s, int base=10);
std::string extractToken(std::string& s);
void deleteLeadOWS(std::string& s);
void deleteTrailOWS(std::string& s);
void deleteQuotedString(std::string& s);
void deleteChunkExt(std::string& s);

void parseThrow(std::string what);
void earlyResponse(Conversation& conv, StatusCode status, bool close=true);
