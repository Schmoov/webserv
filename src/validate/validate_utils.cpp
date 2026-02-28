#include "../../inc/webserv.hpp"

void skipBody(Conversation& conv, StatusCode status) {
	conv.resp.status = status;
	if (!conv.req.header.count("transfer-encoding") && !conv.req.bodyLeft)
		conv.state = EXEC;
	else
		conv.state = PARSE_BODY;
}
