#include "../../inc/execution/execution.hpp"
#include "../../inc/webserv_epoll.hpp"

extern int epoll_fd;

extern std::map<int, Conversation *> conversations_cgi_in;
extern std::map<int, Conversation *> conversations_cgi_out;

void exit_server(const char *error_message)
{
    std::cerr << error_message << strerror(errno) << std::endl;
    std::exit(1);
}

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

std::string createCGIResponse(Conversation &conversation, std::string &raw_output)
{
    std::string status_line = "200 OK";
    std::string separator = "\r\n\r\n";
    size_t sep_index = raw_output.find(separator);
    if(sep_index == std::string::npos)
        return createErrorResponse(INTERNAL_SERVER_ERROR);
    std::string cgi_headers = raw_output.substr(0, sep_index);
    std::string body = raw_output.substr(sep_index + separator.size());
    std::string status_code_string = "Status: ";
    sep_index = cgi_headers.find(status_code_string);
    if(sep_index != std::string::npos)
    {
        size_t start = sep_index + status_code_string.size();
        size_t end = cgi_headers.find("\r\n", sep_index);
        status_line = cgi_headers.substr(start, end - start);
        cgi_headers.erase(sep_index, end - sep_index + 2);
    }

    std::string response = "HTTP/1.0 " + status_line + "\r\n";
        response += "Server: " + conversation.conf->server_name + "\r\n";
        if(conversation.resp.shouldClose)
            response += "Connection: close\r\n";
        else
            response += "Connection: keep-alive\r\n";
    response += cgi_headers + "\r\n\r\n";
    response += body;
    return response;
}

int write_to_pipe(Conversation &conversation, int fd)
{
    ssize_t total = conversation.req.body.size();
    ssize_t written = 0;
    const char *data = conversation.req.body.c_str();

    while(written < total)
    {
        ssize_t size = write(fd, data + written, total - written);
        if(size < 0)
            break;
        written += size;
    }
    return written == total;
}

std::string fork_process(Conversation &conversation)
{
    std::cout << "POST\n";
    int pipe_in[2];
    int pipe_out[2];
    if(pipe(pipe_in) != 0)
        exit_server("Pipe failed");
    if(pipe(pipe_out) != 0)
        exit_server("Pipe failed");

    pid_t pid = fork();

    if(pid < 0)
        exit_server("Fork failed");

    if(pid == 0)
    {
        if(close(pipe_in[1]) != 0)
            _exit(1);
        
        if(close(pipe_out[0]) != 0)
            _exit(1);

        if(dup2(pipe_in[0], STDIN_FILENO) == -1)
            _exit(1);
        if(dup2(pipe_out[1], STDOUT_FILENO) == -1)
            _exit(1);

        if(close(pipe_in[0]) != 0)
            _exit(1);
        if(close(pipe_out[1]) != 0)
            _exit(1);
        
        char *args[] = {const_cast<char *>("/bin/python3"), const_cast<char *>(conversation.req.pathOnDisk.c_str()), NULL};
        std::string path("/bin/python3");
        
        
        int length = conversation.req.body.size();
        std::ostringstream length_str;
        length_str << length;
        std::string content_length = "CONTENT_LENGTH=" + length_str.str();
        std::string content_type;
        if(conversation.req.header.count("content-type"))
            content_type = "CONTENT_TYPE=" + conversation.req.header["content-type"];
        else
            content_type = "CONTENT_TYPE=";
        
        std::string gateway_interface = "GATEWAY_INTERFACE=CGI/1.1";
        std::string query_string;
        if(conversation.req.hasQuery)
            query_string = "QUERY_STRING=" + conversation.req.query;
        else
            query_string = "QUERY_STRING=";
        
        
        ///// NOT ALLOWED
        struct sockaddr_storage address;
        socklen_t address_length = sizeof(address);
        getpeername(conversation.fd, (struct sockaddr *)&address, &address_length);
        
        
        char ip_str[INET6_ADDRSTRLEN];
        
        if (address.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&address;
            inet_ntop(AF_INET, &s->sin_addr, ip_str, sizeof(ip_str));
        } else if (address.ss_family == AF_INET6) {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&address;
            inet_ntop(AF_INET6, &s->sin6_addr, ip_str, sizeof(ip_str));
        }
        
        ///// NOT ALLOWED
        
        std::string remote_address = "REMOTE_ADDR=" + std::string(ip_str);
        std::string remote_host = "REMOTE_HOST=" + std::string(ip_str);
        std::string method = "REQUEST_METHOD=" + conversation.req.method;
        std::string script_name;
        if(conversation.req.pathOnDisk != "")
            script_name = "SCRIPT_NAME=" + conversation.req.pathOnDisk.substr(conversation.req.pathOnDisk.rfind('/') + 1);
        std::string server_name = "SERVER_NAME=" + conversation.conf->host;
        std::string server_port = "SERVER_PORT=" + std::to_string(conversation.conf->port);
        std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.0";
        std::string server_software = "SERVER_SOFTWARE=webserv/1.0";
        
        char *env[] = {
            const_cast<char *>(content_length.c_str()),
            const_cast<char *>(content_type.c_str()),
            const_cast<char *>(gateway_interface.c_str()),
            const_cast<char *>(query_string.c_str()),
            const_cast<char *>(remote_address.c_str()),
            const_cast<char *>(remote_host.c_str()),
            const_cast<char *>(method.c_str()),
            const_cast<char *>(script_name.c_str()),
            const_cast<char *>(server_name.c_str()),
            const_cast<char *>(server_port.c_str()),
            const_cast<char *>(server_protocol.c_str()),
            const_cast<char *>(server_software.c_str()),
            NULL
        };
        execve(path.c_str(), args, env);
        _exit(1); // NOT ALLOWED IN SUBJECT BUT NEEDED ANYWAY. DO WE USE IT OR NOT ?
        // CAN BE REPLACED BY (WHICH IS UGLY AS FUCK)
        //std::abort();
        // OR BUT CAN LEAD TO UNDEFINED BEHAVIOR CAUSE OF STDIO BUFFERS FLUSH (I guess we can be cautious about it)...
        //std::exit(1)
        return "";
    }
    else
    {
        if(close(pipe_out[1]) != 0)
            exit_server("Close failed");
        if(close(pipe_in[0]) != 0)
            exit_server("Close failed");

        int fd_in = pipe_in[1];
        int fd_out = pipe_out[0];

        fcntl(fd_in, F_SETFL, O_NONBLOCK);
        fcntl(fd_out, F_SETFL, O_NONBLOCK);

        struct epoll_event event;
        event.events = EPOLLOUT;
        event.data.fd = fd_in;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_in, &event);
        
        event.events = EPOLLIN;
        event.data.fd = fd_out;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_out, &event);
        
        Cgi &cgi_infos = conversation.resp.cgi_infos;

        cgi_infos.pid = pid;
        cgi_infos.pipe_in = fd_in;
        cgi_infos.pipe_out = fd_out;
        cgi_infos.written = 0;
        cgi_infos.to_write = &conversation.req.body;

        conversations_cgi_in.insert({fd_in, &conversation});
        conversations_cgi_out.insert({fd_out, &conversation});

        return "";
    }
}

std::string handlePost(Conversation &conversation)
{
    if(!conversation.req.pathOnDisk.empty() && conversation.req.pathOnDisk[0] == '/')
        conversation.req.pathOnDisk.erase(0, 1);
    //conversation.req.pathOnDisk = conversation.conf->root + conversation.req.pathOnDisk;
    printf("/////////////////////PATH%s\n", conversation.req.pathOnDisk.c_str());
    if(isFile(conversation.req.pathOnDisk) != OK)
        return (createErrorResponse(NOT_FOUND));
    if(!endsWith(conversation.req.pathOnDisk, ".py")) // REPLACE BY CHECK ON loc.cgiHandler
        return (createErrorResponse(METHOD_NOT_ALLOWED));
    return fork_process(conversation);
}
