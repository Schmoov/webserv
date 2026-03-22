#include "../../inc/execution/execution.hpp"

static std::string createResponse(StatusCode code, std::string content, std::string contentType)
{
    std::ostringstream response;
    response    << "HTTP/1.1 " << code << " " << resolveStatusText(code) << "\r\n"
                << "Content-Type: " << contentType << "\r\n"
                << "Content-Length: " << content.size() << "\r\n"
                << "Connection: Close\r\n" //keep alive
                << "\r\n"
                << content;
    return response.str();
}

static std::string createResponseBody(StatusCode code, bool statusCode)
{
    std::ostringstream response;
    std::string message = resolveStatusText(code);
    response << "<html><body><h1>" << code << " " << message << "</h1></body></html>";
    return createResponse(code, response.str(), "text/html");
}

std::string handleDelete(Conversation &conversation)
{
    Request request = conversation.req;
    std::string uri = request.uri;
    StatusCode code;

    code = isDirectory(uri);
    bool shouldClose = conversation.resp.shouldClose;
    
    if(code == FORBIDDEN || code == INTERNAL_SERVER_ERROR)
        return createResponseBody(code, shouldClose);
    if(code == OK)
    {
        if(rmdir(request.uri.c_str()) != 0)
        {
            if(errno == EACCES)
                return createResponseBody(FORBIDDEN, shouldClose);
            return createResponseBody(INTERNAL_SERVER_ERROR, shouldClose);
        }
        return createResponseBody(OK, shouldClose);
    }

    code = isFile(uri);
    if(code != OK)
        return createResponseBody(code, shouldClose);
    if(std::remove(request.uri.c_str()) != 0)
    {
        if(errno == EACCES)
            return createResponseBody(FORBIDDEN, shouldClose);
        return createResponseBody(INTERNAL_SERVER_ERROR, shouldClose);
    }
    return createResponseBody(OK, shouldClose);
}
