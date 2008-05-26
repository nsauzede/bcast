#include <stdio.h>
#include <string.h>

#include "common.h"

SOCKET create_sock( int port, char *addr)
{
	SOCKET s;

#ifdef _WIN32
	WORD ver;
	WSADATA wsadata;
	ver = MAKEWORD( 2, 2);
	WSAStartup( ver, &wsadata);
#endif
	s = socket( AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sa;
	memset( &sa, 0, sizeof( sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons( port);
	sa.sin_addr.s_addr = inet_addr( addr);
	bind( s, (struct sockaddr *)&sa, sizeof( sa));

	return s;
}

int main( int argc, char* argv[])
{
#define PORT 12345
#define ADDR "127.0.0.1"
	SOCKET sock;
	int port = PORT;
	char *addr = ADDR;
	int arg = 1;

	if (argc > arg)
	{
		addr = argv[arg++];
	}
	printf( "creating sock..\n");
	sock = create_sock( port, addr);
	int n;
	char str[1024];
	printf( "receiving on %d..\n", port);
	n = recv( sock, str, sizeof( str), 0);
	if (n > (sizeof( str) - 1))
		n = sizeof( str) - 1;
	str[n] = 0;
	printf( "recv returned %d [%s]\n", n, str);

	return 0;
}

