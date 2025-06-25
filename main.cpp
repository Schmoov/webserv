#include <iostream>
#include <sys/socket.h>
//#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);
	bind(fd, (struct sockaddr *)&addr, sizeof(addr));

	listen(fd, 69);

	socklen_t sz = sizeof(addr);
	int sock = accept(fd, (struct sockaddr *)&addr, &sz);

	char buf[100000];
	read(sock, buf, sizeof(buf));

	cout << buf << endl;
}
