#pragma once

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

#include "../webserv.hpp"
#include "../config.hpp"

bool file_exist(const std::string& path);

std::string execute(Conversation &conversation);

std::string handleDelete(Conversation &conversation);

std::string handleGet(Conversation &conversation);

std::string handlePost(Conversation &conversation);
int write_to_pipe(Conversation &conversation, int fd);
std::string createCGIResponse(Conversation &conversation, std::string &raw_output);

StatusCode is_path_valid(const std::string& path, struct stat *file_infos);
StatusCode isDirectory(const std::string& path);
StatusCode isFile(const std::string& path);
std::string readFile(const std::string &path);
std::string resolveStatusText(StatusCode code);
bool endsWith(const std::string &str, const std::string &suffix);

std::string createErrorResponse(StatusCode code, bool shouldClose);
std::string createResponse(StatusCode code, std::string content, std::string contentType, bool shouldClose);
std::string connection_status_to_str(bool shouldClose);
