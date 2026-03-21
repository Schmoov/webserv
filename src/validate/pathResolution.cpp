#include "../../inc/validate/Validator.hpp"

using namespace std;
typedef map<string, Location>::iterator locIt;

void Validator::validateUri(Conversation& conv) {
	conv.req.pathOnDisk = conv.req.uri;

	if (conv.req.pathOnDisk[0] != '/')
		stripHost(conv);
	if (conv.state != VALIDATE)
		return;
	vector<string> seg = parsePath(conv);
	if (conv.state != VALIDATE)
		return;
	size_t match = matchLoc(conv, seg);
	if (!match) {
		if (conv.conf->pathToLoc.count("/"))
			conv.loc = &conv.conf->pathToLoc["/"];
		else
			return skipBody(conv, NOT_FOUND);
	}
	assembleUri(conv, seg, match);
}

void Validator::stripHost(Conversation& conv) {
	string& s = conv.req.pathOnDisk;
	toLower(s, 0, 7);
	if (s.compare(0, 7,"http://"))
		return skipBody(conv, BAD_REQUEST);
	s.erase(0, 7);
	if (s.empty())
		s = "/";
	if (s[0] == '/')
		return;
	size_t pos = s.find("/");
	if (pos == npos)
		pos = s.size();
	toLower(s, 0, pos);
	string host = conv.conf->host;
	string hostFull = host + ':' + intToString(conv.conf->port);
	if (s.compare(0, host.size(), host) && s.compare(0, hostFull.size(), hostFull))
		return skipBody(conv, BAD_REQUEST);
	if (!s.compare(0, hostFull.size(), hostFull))
		s.erase(0, hostFull.size());
	else
		s.erase(0, host.size());
	if (!s.size())
		s = "/";
	if (s[0] != '/')
		return skipBody(conv, BAD_REQUEST);
}

vector<string> Validator::parsePath(Conversation& conv) {
	string& s = conv.req.pathOnDisk;
	vector<string> seg;
	while (s.size() && s[0] == '/') {
		while (s.size() && s[0] == '/')
			s.erase(0, 1);
		if (!s.size() || s[0] == '?')
			break;
		size_t pos = s.find_first_of("/?");
		if (pos == npos)
			pos = s.size();
		seg.push_back(s.substr(0, pos));
		s.erase(0, pos);
	}
	if (!seg.size())
		seg.push_back("");
	if (s.size()) {
		s.erase(0,1);
		conv.req.hasQuery = true;
		conv.req.query = s;
	}
	segmentLinter(conv, seg);
	if (conv.state != VALIDATE)
		return seg;
	queryLinter(conv);
	return seg;
}

void Validator::segmentLinter(Conversation& conv, vector<string>& seg) {
	for (size_t i = 0; i < seg.size(); i++) {
		string decoded = "";
		for (size_t j = 0; j < seg[i].size();) {
			if (seg[i][j] == '%') {
				if (j + 2 >= seg[i].size() || base16.find(seg[i][j+1]) == npos
						|| base16.find(seg[i][j+2]) == npos)
					return earlyResponse(conv, BAD_REQUEST);
				string esc = seg[i].substr(j+1,2);
				decoded += peekSize(esc, 16);
				if (decoded[decoded.size()-1] == '/')
					return earlyResponse(conv, BAD_REQUEST);
				j+=2;
			} else {
				if (pchar.find(seg[i][j]) == npos)
					return earlyResponse(conv, BAD_REQUEST);
				decoded += seg[i][j];
				j++;
			}
		}
		seg[i] = decoded;
		if (seg[i] == "." || seg[i] == "..")
			return earlyResponse(conv, BAD_REQUEST);
	}
}

void Validator::queryLinter(Conversation& conv) {
	string& quer = conv.req.query;
	if (quer.size())
		conv.req.uri.erase(conv.req.uri.size() - quer.size() - 1,
				quer.size() + 1);
	for (size_t j = 0; j < quer.size();) {
		if (quer[j] == '%') {
			if (j + 2 >= quer.size() || base16.find(quer[j+1]) == npos
					|| base16.find(quer[j+2]) == npos)
				return earlyResponse(conv, BAD_REQUEST);
			j+=2;
		} else {
			if (qchar.find(quer[j]) == npos)
				return earlyResponse(conv, BAD_REQUEST);
			j++;
		}
	}
}

size_t Validator::matchLoc(Conversation& conv, vector<string>& seg) {
	size_t res = 0;
	for (locIt it = conv.conf->pathToLoc.begin(); it != conv.conf->pathToLoc.end(); it++) {
		size_t curr = 0;
		size_t pos = 0;
		while (pos < it->first.size()) {
			size_t newPos = it->first.find("/", pos+1);
			if (newPos == npos)
				newPos = it->first.size();
			if (seg[curr] == it->first.substr(pos+1, newPos-pos-1))
				curr++;
			else
				break;
			pos = newPos;
		}
		if (curr > res && pos == it->first.size()) {
			res = curr;
			conv.loc = &it->second;
		}
	}
	return res;
}

void Validator::assembleUri(Conversation& conv, vector<string>& seg, size_t match) {
	conv.req.pathOnDisk = "";
	size_t i = 0;
	if (conv.loc->hasRedir) {
		conv.req.uri = conv.loc->redirURL;
		conv.resp.status = conv.loc->redirCode;
		return ;
	}
	else if (conv.loc->hasRoot) {
		conv.req.pathOnDisk += conv.loc->root;
	} else
		conv.req.pathOnDisk += conv.conf->root;

	while (i < seg.size()) {
		conv.req.pathOnDisk += '/' + seg[i];
		i++;
	}
}
