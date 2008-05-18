#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

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
#else
	int i, j;
	static int f = 0;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			dest->r = f+j+i;
			dest->g = f+j+i+1;
			dest->b = f+j+i+2;
			dest++;
		}
	}
	f++;
#endif

	return result;
}

int send_bits( SOCKET sock, void *bits, int size, int width, int height, int frame)
{
	packet_t packet;
	uint16_t seq;
	int left, len;
	
	packet.frame = frame;
	packet.width = width;
	packet.height = height;
	seq = 0;
	left = size;
//	printf( "sending frame %d, dims=%dx%d\n", frame, width, height);
	while (left > 0)
	{
		packet.seq = seq;
		len = left;
		if (len > sizeof(packet.payload))
			len = sizeof( packet.payload);
		memcpy( packet.payload, bits, len);
		send( sock, &packet, sizeof( packet), 0);
		left -= len;
		bits += len;
#if 0
		printf( "sent frame %d seq %d :\n", frame, seq);
		int i;
		for (i =0; i < sizeof( packet); i++)
		{
			printf( "%02X", *((char *)&packet + i));
		}
		printf( "\n");
#endif
		seq++;
	}
//	printf( "sent frame %d, dims=%dx%d\n", frame, width, height);

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
#define W 4
#define H 4
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
	size = sizeof(*dest) * w * h;
	dest = malloc( sizeof( *dest) * w * h);
	int frame = 0;
	while (1)
	{
		get_bits( x, y, w, h, dest);
#if 1
		int i, j;
		printf( "about to send frame %d, dims=%dx%d :\n", frame, w, h);
		for (j = 0; j < h; j++)
		{
			for (i = 0; i < w; i++)
			{
				printf( " %02X:%02X:%02X", dest[j * w + i].r, dest[j * w + i].g, dest[j * w + i].b);
			}
			printf( "\n");
		}
#endif
		send_bits( sock, dest, size, w, h, frame++);
#ifdef _WIN32
		Sleep( 5000);
#else
		sleep( 5);
#endif

	}

	return 0;
}
