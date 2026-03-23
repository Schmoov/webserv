#include "../../inc/execution/execution.hpp"

std::string connection_status_to_str(bool shouldClose)
{
    if(shouldClose)
        return "Connection: close";
    return "Connection: keep-alive";
}

std::string createResponse(StatusCode code, std::string content, std::string contentType, bool shouldClose)
{
    std::cout << "SHOULDCLOSE:" << shouldClose << std::endl;
    std::ostringstream response;
    response    << "HTTP/1.0 " << code << " " << resolveStatusText(code) << "\r\n"
                << "Content-Type: " << contentType << "\r\n"
                << "Content-Length: " << content.size() << "\r\n"
                << "Connection: close" //<< connection_status_to_str(shouldClose) << "\r\n" //keep alive
                << "\r\n"
                << content;
    return response.str();
}

std::string createErrorResponse(StatusCode code, bool shouldClose)
{
    std::cout << "SHOULDCLOSE:" << shouldClose << std::endl;
    std::ostringstream response;
    std::string error_message = resolveStatusText(code);
    response << "<html><body><h1>" << code << " " << error_message << "</h1></body></html>";
    return createResponse(code, response.str(), "text/html", shouldClose);
}