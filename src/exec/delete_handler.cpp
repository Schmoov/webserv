#include "../../inc/execution/execution.hpp"

static std::string createDeleteResponse(StatusCode code, std::string content, std::string contentType, bool shouldClose)
{
    std::ostringstream response;
    std::string connection_header = connection_status_to_str(shouldClose);
    response    << "HTTP/1.1 " << code << " " << resolveStatusText(code) << "\r\n"
                << "Content-Type: " << contentType << "\r\n"
                << "Content-Length: " << content.size() << "\r\n"
                << connection_header << "\r\n" //keep alive
                << "\r\n"
                << content;
    return response.str();
}

static std::string createDeleteResponseBody(StatusCode code, bool shouldClose)
{
    std::ostringstream response;
    std::string message = resolveStatusText(code);
    response << "<html><body><h1>" << code << " " << message << "</h1></body></html>";
    return createResponse(code, response.str(), "text/html", shouldClose);
}

std::string handleDelete(Conversation &conversation)
{
    Request request = conversation.req;
    std::string uri = request.uri;
    StatusCode code;

    code = isDirectory(uri);
    bool shouldClose = conversation.resp.shouldClose;
    
    if(code == FORBIDDEN || code == INTERNAL_SERVER_ERROR)
        return createDeleteResponseBody(code, shouldClose);
    if(code == OK)
    {
        if(rmdir(request.uri.c_str()) != 0)
        {
            if(errno == EACCES)
                return createDeleteResponseBody(FORBIDDEN, shouldClose);
            return createDeleteResponseBody(INTERNAL_SERVER_ERROR, shouldClose);
        }
        return createDeleteResponseBody(OK, shouldClose);
    }

    code = isFile(uri);
    if(code != OK)
        return createDeleteResponseBody(code, shouldClose);
    if(std::remove(request.uri.c_str()) != 0)
    {
        if(errno == EACCES)
            return createDeleteResponseBody(FORBIDDEN, shouldClose);
        return createDeleteResponseBody(INTERNAL_SERVER_ERROR, shouldClose);
    }
    return createDeleteResponseBody(OK, shouldClose);
}
