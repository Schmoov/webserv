#include <asm-generic/socket.h>
#include <iostream>
#include <sys/socket.h>
//#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	sockaddr_in addr {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)))
		cout << "FAIL\n";

	listen(fd, 69);

	socklen_t sz = sizeof(addr);
	int sock = accept(fd, (struct sockaddr *)&addr, &sz);

	char buf[100000];
	while(1) {
	read(sock, buf, sizeof(buf));

	cout << buf << endl;

	string resp =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello, world!";

	write(sock, resp.c_str(), resp.size());
	}
}
