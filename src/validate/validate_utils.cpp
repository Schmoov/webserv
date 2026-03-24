#include "../../inc/webserv.hpp"

void skipBody(Conversation& conv, StatusCode status) {
    conv.resp.status = status;
    conv.resp.shouldClose = true;
    conv.state = EXEC;
}
