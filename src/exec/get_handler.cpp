#include "../../inc/execution/execution.hpp"

static std::string directoryListing(const std::string& path, bool shouldClose)
{
    DIR *directory = opendir(path.c_str());
    if(!directory)
        return createErrorResponse(INTERNAL_SERVER_ERROR, shouldClose);
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
    return createResponse(OK, htmlListing.str(), "text/html", shouldClose);
}

static std::string getMime(std::string path)
{
    std::size_t pos = path.find_last_of('.');
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
    Request request = conversation.req;
    std::string pathOnDisk = request.pathOnDisk;
    StatusCode code;
    bool shouldClose = conversation.resp.shouldClose;

    code = isDirectory(pathOnDisk);
    if(code == FORBIDDEN || code == INTERNAL_SERVER_ERROR)
        return createErrorResponse(code, shouldClose);
    if(code == OK)
    {
        std::string path_with_index = pathOnDisk;
        if (path_with_index[path_with_index.size() - 1] != '/')
            path_with_index += "/";
        path_with_index += "index.html";

        if(isFile(path_with_index) == OK)
        {
            std::string content = readFile(path_with_index);
            return createResponse(OK, content, getMime(path_with_index), shouldClose);
        }

        if(conversation.loc->autoIndex)
            return directoryListing(pathOnDisk, shouldClose);

        return createErrorResponse(FORBIDDEN, shouldClose);
    }

    code = isFile(pathOnDisk);
    if(code != OK)
        return createErrorResponse(code, shouldClose);
    if(endsWith(pathOnDisk, ".py")) // REPLACE BY CHECK ON loc.cgiHandler
        return (createErrorResponse(METHOD_NOT_ALLOWED, shouldClose));

    std::string content = readFile(pathOnDisk);
    std::string contentType = getMime(pathOnDisk);

    return createResponse(OK, content, getMime(pathOnDisk), shouldClose);
}
