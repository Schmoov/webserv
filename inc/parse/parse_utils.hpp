#pragma once
#include <map>
#include <string>

#include "../webserv.hpp"

const std::string tchar = alpha + "!#$%&'*+-.^_|~";

size_t peekSize(std::string& s, int base=10);
std::string extractToken(std::string& s);
void deleteLeadOWS(std::string& s);
void deleteTrailOWS(std::string& s);
void deleteQuotedString(std::string& s);
void deleteChunkExt(std::string& s);

void parseThrow(std::string what);
void earlyResponse(Conversation& conv, StatusCode status, bool close=true);
