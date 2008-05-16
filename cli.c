#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

SOCKET create_sock( int port, char *addr)
{
	SOCKET s;

	s = socket( AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sa;
	memset( &sa, 0, sizeof( sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons( port);
	sa.sin_addr.s_addr = inet_addr( addr);
	bind( s, (struct sockaddr *)&sa, sizeof( sa));

	return s;
}

int main()
{
#define PORT 12345
#define ADDR "127.0.0.1"
	SOCKET sock;
	int port = PORT;
	char *addr = ADDR;

	sock = create_sock( port, addr);
	while (1)
	{
		int n;
		char buf[1024];

		n = recv( sock, buf, sizeof( buf), 0);
		if (n == -1)
		{
			Sleep( 1000);
			continue;
		}
		printf( "received %d\n", n);
	}
	return 0;
}
