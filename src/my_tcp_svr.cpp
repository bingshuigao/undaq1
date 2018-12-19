#include "my_tcp_svr.h"
#include <string.h>
#include <arpa/inet.h>

int my_tcp_svr::init(int port, int backlog, std::string* err)
{
	int on = 1;
	struct sockaddr_in servaddr;

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		if (err)
			*err = "error creating socket";
		return -1;
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* init servaddr */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	/* bind and listen */
	if (bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		if (err)
			*err = "error binding";
		return -1;
	}
	if (listen(sock, backlog) == -1) {
		if (err) 
			*err = "error listening";
		return -1;
	}

	return 0;
}
