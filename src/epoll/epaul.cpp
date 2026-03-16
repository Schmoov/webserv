#include "../../inc/webserv.hpp"
#include "../../inc/read/Reader.hpp"
#include "../../inc/parse/Parser.hpp"
#include "../../inc/validate/Validator.hpp"

#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h> // FOR init_addr DO NOT SHIP WITH IT !
#include <fcntl.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>

#define PORT        8080
#define MAX_EVENTS  10

#include <signal.h>

volatile int keep_running = 1;

#include <list>

int epoll_fd;
struct epoll_event event;

std::map<int, Conversation> conversations;
std::map<int, Conversation *> conversations_cgi_in;
std::map<int, Conversation *> conversations_cgi_out;

void stateManager(Conversation& conv);

Conversation::Conversation() {
		parser = new Parser;
		reader = new Reader;
		validator = new Validator;
	}

std::map<int, ServerConfig> sConf;
static bool addNewConnection(int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(server_fd,
        (struct sockaddr *)&client_addr, &len);

    if (client_fd == -1)
        return false;

    fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL) | O_NONBLOCK);

    event.events  = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

    Conversation conversation;
    conversation.fd = client_fd;
    conversations.insert({client_fd, conversation});
    conversations[client_fd].conf = &sConf[PORT];

    printf("\t[NEW]  Client connected! fd=%d\n", client_fd);
    return true;
}

void execute_place_holder(int fd)
{
    printf("[EXECUTE]\n");
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    // EXECUTE PLACE HOLDER
    // EXECUTE CGI
    // END EXECUTE
    conversations[fd].state = WRITE_CLIENT;
    event.events  = EPOLLOUT;
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}


#include "../../inc/configParse/configParse.hpp"
int main(int argc, char** argv)
{
    Parser parser;

    sConf = parseConfig(argv[1]);

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);
    fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL) | O_NONBLOCK);
    printf("[SETUP] Server listening on port %d\n", PORT);

    // Epoll
    epoll_fd = epoll_create1(0);
    printf("[SETUP] epoll_fd = %d\n\n", epoll_fd);

    
    event.events  = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
    printf("[SETUP] Added server_fd=%d to epoll (watching EPOLLIN)\n\n", server_fd);

    struct epoll_event events[MAX_EVENTS];

    printf("=== Entering event loop\n\n");

    while (keep_running)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 200);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        if(n != 0)
            printf("\n\n\n[WAKE] epoll returned %d event(s)!\n\n", n);
        
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            printf("[EVENT] on fd %d\n", fd);

            //New connection
            if (fd == server_fd)
            {
                if(!addNewConnection(server_fd))
                    continue;
            }
            // Client FD
            else 
            {
                if (events[i].events & EPOLLIN)
                {
                    
                    printf("\t[TRIGGER]  EPOLLIN fd=%d\n", fd);

                    printf("\t\t[READ]  CLIENT READ fd=%d\n", fd);
                    conversations[fd].state = READ_CLIENT;
                    manage(conversations[fd]);
                    if (conversations[fd].state == EXEC) {
                        execute_place_holder(fd);
                        continue;
                    }
                }
                else if(events[i].events & EPOLLOUT)
                {
                    printf("[EPOLLOUT] send and exiting\n");
                    


                    printf("////////DEBUG START///////\n");
                    printf("Method:%s\n", conversations[fd].req.method.c_str());
                    printf("Uri:%s\n", conversations[fd].req.uri.c_str());
                    for(const auto &pair : conversations[fd].req.header)
                        printf("Headers:%s %s\n", pair.first.c_str(), pair.second.c_str());
                    printf("PathOnDisk:%s\n", conversations[fd].req.pathOnDisk.c_str());
                    printf("Body:%s\n", conversations[fd].req.body.c_str());
                    printf("HasQuery:%d\n", conversations[fd].req.hasQuery);
                    printf("Query:%s\n", conversations[fd].req.query.c_str());
                    printf("State:%d\n", conversations[fd].state);
                    printf("Status:%d\n", conversations[fd].resp.status);
                    printf("////////DEBUG END///////\n");


                    send(fd, conversations[fd].buf.c_str(), conversations[fd].buf.size(), 0);
                    // If KEEP ALIVE ->
                    /*event.events  = EPOLLIN;
                    event.data.fd = server_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, server_fd, &event);
                    else*/
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    conversations.erase(fd);
                }
            }

            if(events[i].events & (EPOLLHUP | EPOLLERR))
            {
                printf("[EPOLLHUP | EPOLLERR] Something probably went wrong\n");
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
                close(fd);
                conversations.erase(fd);
            }
        }
    }

    close(epoll_fd);
    close(server_fd);
    return 0;
}
