#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
typedef int SOCKET;
#endif

typedef struct pixel {
	int r, g, b;
} pixel_t;

int get_bits( int x, int y, int w, int h, pixel_t *dest)
{
	int result = 0;

	if (!dest)
		return -1;
#ifdef _WIN32
	HWND hwnd;
	POINT p;
	p.x = x;
	p.y = y;
	hwnd = WindowFromPoint( p);
	if (hwnd == NULL)
	{
		printf( "%s: Failed to get HWND at (%d;%d)\n", __func__, x, y);
	}
	else
	{
		char title[1024] = "";

		GetWindowText( hwnd, title, sizeof( title));
//		printf( "%s: got HWND=%p at (%d;%d) [%s]\n", __func__, hwnd, x, y, title);
		RECT rect;
		if (!GetClientRect( hwnd, &rect))
		{
			printf( "%s: Failed to get rect of HWND=%p\n", __func__, hwnd);
		}
		else
		{
//			printf( "%s: got rect=(%ld;%ld;%ld;%ld)\n", __func__, rect.left, rect.right, rect.top, rect.bottom);
			HDC hdc;
			hdc = GetDC( hwnd);
			if (hdc == NULL)
			{
				printf( "%s: Failed to get HDC of HWND=%p\n", __func__, hwnd);
			}
			else
			{
//				printf( "%s: got HDC=%p\n", __func__, hdc);
				x = 0;
				y = 0;
				COLORREF cref;
				int i, j;
				for (j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
					{
						cref = GetPixel( hdc, x + i, y + j);
						dest->r = GetRValue( cref);
						dest->g = GetGValue( cref);
						dest->b = GetBValue( cref);
						dest++;
//						printf( " %08lx", cref);
					}
//					printf( "\n");
				}
			}
		}
	}
#endif

	return result;
}

int send_bits( SOCKET sock, pixel_t *bits, int size)
{
	send( sock, (void *)size, sizeof( size), 0);
	send( sock, (void *)bits, size, 0);

	return 0;
}

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
	int on = 1;
	setsockopt( s, SOL_SOCKET, SO_BROADCAST, (void *)&on, sizeof( on));
	struct sockaddr_in sa;
	memset( &sa, 0, sizeof( sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons( port);
	sa.sin_addr.s_addr = inet_addr( addr);
	connect( s, (struct sockaddr *)&sa, sizeof( sa));

	return s;
}

int main()
{
#define W 15
#define H 15
#define X 10
#define Y 10
#define ADDR "127.0.0.1"
#define PORT 12345
	pixel_t *dest = NULL;
	int size;
	int x = X;
	int y = Y;
	int w = W;
	int h = H;
	SOCKET sock;
	int port = PORT;
	char *addr = ADDR;

	sock = create_sock( port, addr);
	size = sizeof( *dest) * w * h;
	dest = malloc( sizeof( *dest) * w * h);
	while (1)
	{
		get_bits( x, y, w, h, dest);
		send_bits( sock, dest, size);
		Sleep( 25);

#if 0
		int i, j;
		for (j = 0; j < h; j++)
		{
			for (i = 0; i < w; i++)
			{
				printf( " %02X:%02X:%02X", dest[j * w + i].r, dest[j * w + i].g, dest[j * w + i].b);
			}
			printf( "\n");
		}
#endif
	}

	return 0;
}
