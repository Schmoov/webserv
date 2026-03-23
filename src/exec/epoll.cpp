#include "../../inc/webserv_epoll.hpp"
#include "../../inc/webserv.hpp"
#include "../../inc/execution/execution.hpp"
#include "../../inc/read/Reader.hpp"
#include "../../inc/parse/Parser.hpp"
#include "../../inc/configParse/configParse.hpp"
#include "../../inc/validate/Validator.hpp"

#define MAX_EVENTS  10

#include <signal.h>

volatile int keep_running = 1;

#include <list>

int epoll_fd;
struct epoll_event event;

std::map<int, Conversation> conversations;
std::map<int, Conversation *> conversations_cgi_in;
std::map<int, Conversation *> conversations_cgi_out;

std::map<int, ServerConfig> server_configs;

void handle_sigint(int sig)
{
    (void)sig;
    printf("\n[SHUTDOWN] Caught Ctrl+C, shutting down...\n");
    keep_running = 0;
}

std::string adress_to_string(uint32_t adress)
{
    std::stringstream ip;
    ip << ((adress >> 24) & 0xFF) << "." << ((adress >> 16) & 0xFF) << "." << ((adress >> 8) & 0xFF) << "." << ((adress) & 0xFF);
    return ip.str();
}

static bool addNewConnection(int server_fd, ServerConfig &configuration)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(server_fd,
        (struct sockaddr *)&client_addr, &len);

    if (client_fd == -1)
        return false;

    uint32_t client_adress = ntohl(client_addr.sin_addr.s_addr);

    fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL) | O_NONBLOCK);

    event.events  = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

    Conversation conversation;
    conversation.fd = client_fd;
    conversations.insert(std::make_pair(client_fd, conversation));
    conversations[client_fd].conf = &configuration;

    conversations[client_fd].state = READ_CLIENT;
    conversations[client_fd].client_adress = adress_to_string(client_adress);

    printf("\t[NEW]  Client connected! Client adress is:%s on fd=%d\n", conversations[client_fd].client_adress.c_str(), client_fd);
    return true;
}

static bool readCgiOutput(int cgi_fd, Conversation &conversation)
{
    Cgi &cgi_infos = conversation.resp.cgi_infos;
    char buffer[50];
    ssize_t r;
    r = read(cgi_fd, buffer, sizeof(buffer));
    cgi_infos.raw_output.append(buffer, r);
    printf("\t\t[READ] buffer:%s\n", buffer);
    if(r == 0)
        return true;
    return false;
}

static void endReadCgiOutput(Conversation &conversation, Cgi &cgi_infos, int cgi_fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cgi_fd, NULL);
    close(cgi_fd);
    conversations_cgi_out.erase(cgi_fd);

    int status;

    waitpid(cgi_infos.pid, &status, 0);
    Response &response = conversation.resp;
    if((WIFEXITED(status) && WEXITSTATUS(status) != 0))
        response.content = createErrorResponse(INTERNAL_SERVER_ERROR, conversation.resp.shouldClose);
    else
        response.content = createCGIResponse(conversation, cgi_infos.raw_output);
    response.content_size = response.content.size();

    event.events = EPOLLOUT;
    event.data.fd = conversation.fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conversation.fd, &event);
}

bool drain_cgi(int fd)
{
    if(conversations_cgi_out.count(fd))
    {
        printf("\t\t[READ]  CGI READ EPOLLHUP fd=%d\n", fd);
        Conversation *conversation = conversations_cgi_out[fd];
        Cgi &cgi_infos = conversation->resp.cgi_infos;
        
        printf("\t\t[READ]  CGI READ fd=%d associated with client %d\n", fd, conversation->fd);
        if(readCgiOutput(fd, *conversation))
        {
            printf("[END_CGI_READ] fd:%d, client fd:%d\n", fd, conversation->fd);
            endReadCgiOutput(*conversation, cgi_infos, fd);
        }
        return true;
    }
    return false;
}

bool read_cgi(int fd)
{
    if(conversations_cgi_out.count(fd))
    {
        Conversation *conversation = conversations_cgi_out[fd];
        Cgi &cgi_infos = conversation->resp.cgi_infos;
        
        printf("\t\t[READ]  CGI READ fd=%d associated with client %d\n", fd, conversation->fd);
        if(readCgiOutput(fd, *conversation))
        {
            printf("[END_CGI_READ] fd:%d, client fd:%d", fd, conversation->fd);
            endReadCgiOutput(*conversation, cgi_infos, fd);
        }
        return true;
    }
    return false;
}

void close_conversation_cgi(int fd)
{
    printf("[END CONVERSATION] EPOLLHUP|EPOLLERR client fd:%d\n", fd);
    if(conversations_cgi_in.count(fd))
        conversations_cgi_in.erase(fd);
    if(conversations_cgi_out.count(fd))
        conversations_cgi_out.erase(fd);
    if(conversations.count(fd))
        conversations.erase(fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

bool send_and_close_if_needed(int fd)
{
    printf("[EPOLLOUT] send and check close\n");
    send(fd, conversations[fd].resp.content.c_str(), conversations[fd].resp.content.size(), 0);
    if(conversations[fd].resp.shouldClose)
    {
        printf("[CLOSE] fd:%d\n", fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        conversations.erase(fd);
        return true;
    }
    event.events  = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    conversations[fd].state = PARSE;
    return false;
}

void print_conversation_infos(int fd)
{
    printf("////////DEBUG START///////\n");
    printf("Method:%s\n", conversations[fd].req.method.c_str());
    printf("Uri:%s\n", conversations[fd].req.uri.c_str());
    for (std::map<std::string, std::string>::const_iterator it = conversations[fd].req.header.begin();
            it != conversations[fd].req.header.end(); ++it)
        printf("Headers:%s %s\n", it->first.c_str(), it->second.c_str());
    printf("PathOnDisk:%s\n", conversations[fd].req.pathOnDisk.c_str());
    printf("Body:%s\n", conversations[fd].req.body.c_str());
    printf("HasQuery:%d\n", conversations[fd].req.hasQuery);
    printf("Query:%s\n", conversations[fd].req.query.c_str());
    printf("State:%d\n", conversations[fd].state);
    printf("Status Code:%d\n", conversations[fd].resp.status);
    printf("Should Close:%d\n", conversations[fd].resp.shouldClose);
    printf("////////DEBUG END///////\n");
}

void no_cgi_response(int fd, std::string content)
{
    printf("\t\t[SEND] NO CGI,  Response fd=%d\n", fd);
    conversations[fd].resp.content = content;
    conversations[fd].resp.content_size = content.size();

    event.events = EPOLLOUT;
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

void execute(int fd)
{
    print_conversation_infos(fd); //<<-- debug
    std::string response_content = execute(conversations[fd]);
    printf("\t\t[EXECUTE]  returned:%s fd=%d\n", response_content.c_str(), fd);
    // If empty, then CGI happened so we just wait for CGI to finish
    // We don't go further
    if(response_content.empty())
    {
        printf("\t\t[WAIT_CGI]  client fd:%d CGI fd_in:%d fd_out:%d\n", fd, conversations[fd].resp.cgi_infos.pipe_in, conversations[fd].resp.cgi_infos.pipe_out);
        return;
    }
    no_cgi_response(fd, response_content);
}

bool read_client(int fd)
{
    printf("\t\t[READ]  CLIENT READ fd=%d\n", fd);
    manage(conversations[fd]);
    if(conversations[fd].state == FINISH)
    {
        printf("[CLOSE] fd:%d\n", fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        conversations.erase(fd);
        return false;
    }
    if (conversations[fd].state == EXEC) 
    {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        execute(fd);
        return true;
    }
    return false;
}

bool write_cgi(int fd)
{
    if(conversations_cgi_in.count(fd))
    {
        printf("\t\t[WRITE]  CGI fd=%d\n", fd);
        Conversation *conversation = conversations_cgi_in[fd];
        Cgi &cgi = conversation->resp.cgi_infos;
        std::string &to_write = *cgi.to_write;
        const char *left_to_write = to_write.c_str() + cgi.written;
        size_t length = to_write.size() - cgi.written;

        ssize_t n = write(fd, left_to_write, length);
        if(n > 0)
            cgi.written += n;

        if(cgi.written >= cgi.to_write->size() || n < 0)
        {
            printf("[END_CGI_WRITE] fd:%d, client fd:%d\n", fd, conversation->fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            conversations_cgi_in.erase(fd);
        }
        return true;
    }
    return false;
}

int create_server_socket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = /*INADDR_ANY*/inet_addr("127.0.0.1");

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);
    fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL) | O_NONBLOCK);
    printf("[SETUP] Server listening on port %d\n", port);
    return server_fd;
}

void add_server_port_to_epoll(int server_fd)
{
    printf("[SETUP] epoll_fd = %d\n\n", epoll_fd);

    event.events  = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
    printf("[SETUP] Added server_fd=%d to epoll (watching EPOLLIN)\n\n", server_fd);
}

bool is_server_connection(int event_fd)
{
    for (std::map<int, ServerConfig>::iterator it = server_configs.begin();
            it != server_configs.end(); ++it)
    {
        int server_fd = it->first;
        if (event_fd == server_fd)
        {
            addNewConnection(server_fd, it->second);
            return true;
        }
    }
    return false;
}

void main_loop()
{
    printf("=== Entering event loop\n\n");
    struct epoll_event events[MAX_EVENTS];

    while (keep_running)
    {
        int number_of_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);

        if (number_of_events == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        if(number_of_events != 0)
            printf("\n\n\n[WAKE] epoll returned %d event(s)!\n\n", number_of_events);
        
        for (int i = 0; i < number_of_events; i++)
        {
            int fd = events[i].data.fd;
            printf("[EVENT] on fd %d\n", fd);
            if(is_server_connection(fd))
                continue;
            
            if (events[i].events & EPOLLIN)
            {
                printf("\t[TRIGGER]  EPOLLIN fd=%d\n", fd);
                if(read_cgi(fd))
                    continue;
                if(read_client(fd))
                    continue;
            }
            else if(events[i].events & EPOLLOUT)
            {
                printf("\t[TRIGGER]  EPOLLOUT fd=%d\n", fd);
                if(write_cgi(fd))
                    continue;

                if(!send_and_close_if_needed(fd))
                    conversations[fd].state = READ_CLIENT;
            }
            if(events[i].events & (EPOLLHUP | EPOLLERR))
            {
                printf("\t[TRIGGER]  EPOLLHUP | EPOLLERR fd=%d\n", fd);
                if(drain_cgi(fd))
                        continue;
                close_conversation_cgi(fd);
            }
        }
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    signal(SIGINT, handle_sigint);
    std::map<int, ServerConfig> configurations;
    
    configurations = parseConfig(argv[1]);
    epoll_fd = epoll_create1(0);
    for (std::map<int, ServerConfig>::const_iterator it = configurations.begin();
            it != configurations.end(); ++it)
    {
        int server_fd = create_server_socket(it->second.port);
        server_configs[server_fd] = it->second;
        add_server_port_to_epoll(server_fd);
    }
    
    main_loop();

    for (std::map<int, ServerConfig>::const_iterator it = server_configs.begin();
            it != server_configs.end(); ++it)
        close(it->first);
    
    close(epoll_fd);
    return 0;
}
