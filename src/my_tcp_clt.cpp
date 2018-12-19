#include "my_tcp_clt.h"
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int my_tcp_clt::connect(int port, const char* host_name, std::string* err)
{
	int sock;
	struct sockaddr_in server_addr;
	struct hostent *host;
	
	host = gethostbyname(host_name);
	if (!host) {
		if (err)
			*err = "cannot get host name";
		return -1;
	}
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock == -1) {
		if (err)
			*err = "cannot create socket";
	}

	/* fill the server info */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(port);
	server_addr.sin_addr=*((struct in_addr *)host->h_addr);

	/* establish the connection */
	if (::connect(sock, (struct sockaddr *)(&server_addr), 
				sizeof(struct sockaddr)) == -1) {
		if (err)
			*err = "cannot connect";
		return -1;
	}

	/* if everything goes fine, return the socket */
	return sock;
}
