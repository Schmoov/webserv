#include "../../inc/execution/execution.hpp"

bool endsWith(const std::string &str, const std::string &suffix)
{
    if (suffix.size() > str.size())
        return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

StatusCode is_path_valid(const std::string& path, struct stat *file_infos)
{
    if(stat(path.c_str(), file_infos) != 0)
    {
        if(errno == ENOENT)
        {
            std::cout << "Path " << path << " doesn't exists" << std::endl;
            return NOT_FOUND;
        }
        else if(errno == EACCES)
        {
            std::cout << "Permission needed" << std::endl;
            return FORBIDDEN;
        }
        else
        {
            std::cout << "Something bad happened" << std::endl;
            return INTERNAL_SERVER_ERROR;
        }
    }
    return OK;
}

StatusCode isDirectory(const std::string& path)
{
    struct stat file_infos;

    StatusCode code = is_path_valid(path, &file_infos);
    if(code != OK)
        return code;
    if(!S_ISDIR(file_infos.st_mode))
        return NOT_A_STATUS_CODE;
    if(access(path.c_str(), R_OK | X_OK) != 0)
    {
        std::cout << "Permission needed" << std::endl;
        return FORBIDDEN;
    }
    return OK;
}

StatusCode isFile(const std::string& path)
{
    struct stat file_infos;

    StatusCode code = is_path_valid(path, &file_infos);
    if(code != OK)
        return code;
    if(!S_ISREG(file_infos.st_mode))
        return NOT_FOUND;
    if(access(path.c_str(), R_OK) != 0)
    {
        std::cout << "Permission needed" << std::endl;
        return FORBIDDEN;
    }
    return OK;
}

std::string readFile(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::binary);

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::string resolveStatusText(StatusCode code)
{
    switch(code)
    {
        case OK:
            return "OK";
        case CONTINUE:
            return "Continue";
        case CREATED:
            return "Created";
        case NO_CONTENT:
            return "No Content";
        case MOVED_PERMANENTLY:
            return "Moved Permanently";
        case FOUND:
            return "Found";
        case BAD_REQUEST:
            return "Bad Request";
        case FORBIDDEN:
            return "Forbidden";
        case NOT_FOUND:
            return "Not Found";
        case METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case TIMEOUT:
            return "Request Time-out";
        case LENGTH_REQUIRED:
            return "Length Required";
        case ENTITY_TOO_LARGE:
            return "Entity Too Large";
        case URI_TOO_LONG:
            return "URI Too Long";
        case EXPECTATION_FAILED:
            return "Expectation Failed";
        case REQUEST_HEADER_FIELDS_TOO_LARGE:
            return "Request Header Fields Too Large";
        case INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case NOT_IMPLEMENTED:
            return "Not Implemented";
        case HTTP_VERSION_NOT_SUPPORTED:
            return "HTTP Version Not Supported";
        default:
            return "Not a code";
    }
}