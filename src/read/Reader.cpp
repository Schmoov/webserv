#include "../../inc/read/Reader.hpp"
#include <string>
#include <unistd.h>
#include <iostream>

using namespace std;

//read failing means the client has left
//read returning 0 means we encounter EOF
//either we successfully served or request is malformed
void Reader::read(Conversation& conv) {
	char toAdd[BUFFER_SIZE];
	int byteRead = ::read(conv.fd, toAdd, BUFFER_SIZE);
	if (byteRead == -1) {
		conv.state = FINISH;
		return;
	}
	if (byteRead == 0) {
		conv.state = EOF_CLIENT;
		return;
	}
	conv.buf.append( toAdd, byteRead);
	conv.state = PARSE;
}
