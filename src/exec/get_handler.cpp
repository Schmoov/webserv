#include "../../inc/execution/execution.hpp"

static std::string createResponse(StatusCode code, std::string content, std::string contentType)
{
    std::ostringstream response;
    response    << "HTTP/1.0 " << code << " " << resolveStatusText(code) << "\r\n"
                << "Content-Type: " << contentType << "\r\n"
                << "Content-Length: " << content.size() << "\r\n"
                << "Connection: Close\r\n" //keep alive
                << "\r\n"
                << content;
    return response.str();
}

static std::string createErrorResponse(StatusCode code)
{
    std::ostringstream response;
    std::string error_message = resolveStatusText(code);
    response << "<html><body><h1>" << code << " " << error_message << "</h1></body></html>";
    return createResponse(code, response.str(), "text/html");
}

static std::string directoryListing(const std::string& path)
{
    DIR *directory = opendir(path.c_str());
    if(!directory)
        return createErrorResponse(INTERNAL_SERVER_ERROR);
    std::ostringstream htmlListing;
    htmlListing << "<html><body><h1>" << path << "</h1><ul>";
    struct dirent *entry;
    while(true)
    {
        entry = readdir(directory);
        if(entry == NULL)
            break;
        std::string name = entry->d_name;
        htmlListing << "<li><a href=\"" << path;
        if(path[path.size() - 1] != '/')
            htmlListing << "/";
        htmlListing << name << "\">" << name << "</a></li>";
    }
    closedir(directory);
    htmlListing << "</ul></body></html>";
    return createResponse(OK, htmlListing.str(), "text/html");
}

static std::string getMime(std::string path)
{
    int pos = path.find_last_of('.');
    if(pos == std::string::npos)
        return "application/octet-stream";
    if(pos + 1 >= path.size())
        return "application/octet-stream";
    std::string extension = path.substr(pos + 1);
    if(extension == "html")
        return "text/html";
    if(extension == "png")
        return "image/png";
    else
        return "application/octet-stream";
}

std::string handleGet(Conversation &conversation)
{
    if(!conversation.req.pathOnDisk.empty() && conversation.req.pathOnDisk[0] == '/')
        conversation.req.pathOnDisk.erase(0, 1);
    conversation.req.pathOnDisk = conversation.conf->root + conversation.req.pathOnDisk;
    printf("/////////////////////PATH%s\n", conversation.req.pathOnDisk.c_str());
    Request request = conversation.req;
    std::string pathOnDisk = request.pathOnDisk;
    StatusCode code;

    code = isDirectory(pathOnDisk);
    if(code == FORBIDDEN || code == INTERNAL_SERVER_ERROR)
        return createErrorResponse(code);
    if(code == OK)
    {
        std::string path_with_index = pathOnDisk;
        if (path_with_index[path_with_index.size() - 1] != '/')
            path_with_index += "/";
        path_with_index += "index.html";

        if(isFile(path_with_index) == OK)
        {
            std::string content = readFile(path_with_index);
            return createResponse(OK, content, getMime(path_with_index));
        }

        if(conversation.loc->autoIndex)
            return directoryListing(pathOnDisk);

        return createErrorResponse(FORBIDDEN);
    }

    code = isFile(pathOnDisk);
    if(code != OK)
        return createErrorResponse(code);
    if(endsWith(pathOnDisk, ".py")) // REPLACE BY CHECK ON loc.cgiHandler
        return (createErrorResponse(METHOD_NOT_ALLOWED));

    std::string content = readFile(pathOnDisk);
    std::string contentType = getMime(pathOnDisk);

    return createResponse(OK, content, getMime(pathOnDisk));
}
