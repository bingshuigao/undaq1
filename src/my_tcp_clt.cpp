#include "my_tcp_clt.h"
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

/* The code is mostly copied from the man(3) page of getaddrinfo */
int my_tcp_clt::connect(int port, const char* host_name, std::string* err)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, j;
	char service[100];
	
	/* Obtain address(es) matching host/port */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */
	
	sprintf(service, "%d", port);
again:
	s = getaddrinfo(host_name, service, &hints, &result);
	if (s != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	    std::cout<<host_name<<service<<std::endl;
	    goto again;
	    if (err)
	     	*err = "cannot get hostname ";
	    return -1;
	}
	
	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully connect(2).
	   If socket(2) (or connect(2)) fails, we (close the socket
	   and) try the next address. */
	
	for (rp = result; rp != NULL; rp = rp->ai_next) {
	    sfd = socket(rp->ai_family, rp->ai_socktype,
	                 rp->ai_protocol);
	    if (sfd == -1)
	        continue;
	
	    if (::connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
	        break;                  /* Success */
	    close(sfd);
	}
	
	if (rp == NULL) {               /* No address succeeded */
	    fprintf(stderr, "Could not connect\n");
	    if (err)
	     	*err = "cannot get connect ";
	    return -1;
	}
	//freeaddrinfo(result);           /* No longer needed */
	return sfd;
}
