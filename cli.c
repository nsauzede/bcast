#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include "common.h"

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

// if error, returns -1
// if dims changed; returns 1
int recv_bits( SOCKET sock, void *dest, int cur_width, int cur_height, int *width, int *height, int *frame)
{
	int result = 0;
	packet_t packet;
	int left, len;
	int first = 1;
	
	left = cur_width * cur_height * sizeof( pixel_t);
	while (left > 0)
	{
		int n;
		
		n = recv( sock, &packet, sizeof(packet), 0);
		if (n == -1)
		{
#ifdef _WIN32
			Sleep( 1000);
#else
			sleep( 1);
#endif
			continue;
		}
//		printf( "received packet : frame=%d seq=%d dims=%dx%d\n", packet.frame, packet.seq, packet.width, packet.height);
		if (width && height)
		{
			*width = packet.width;
			*height = packet.height;
			if ((*width != cur_width) || (*height != cur_height))
			{
				result = 1;
				break;
			}
		}
		if (first)
		{
			if (packet.seq != 0)
				continue;
			first = 0;
			if (frame)
				*frame = packet.frame;
		}
#if 1
		printf( "receiving frame %d seq %d :\n", packet.frame, packet.seq);
		int i;
		for (i = 0; i < sizeof( packet); i++)
		{
			printf( "%02X", *((char *)&packet + i));
		}
		printf( "\n");
#endif
		len = sizeof(packet.payload);
		if (dest)
		{
			memcpy( dest, packet.payload, len);
			dest += len;
		}
		left -= len;
	}
	
	return result;
}

int main()
{
#define PORT 12345
#define ADDR "127.0.0.1"
	SOCKET sock;
	int port = PORT;
	char *addr = ADDR;
	pixel_t *bits = NULL;
	int width = 1;
	int height = 1;

	sock = create_sock( port, addr);
	while (1)
	{
		int n;
		int w, h, f;

		n = recv_bits( sock, bits, width, height, &w, &h, &f);
		if (n == -1)
		{
			break;
		}
		if (n == 1)
		{
			printf( "changing dims from %dx%d to %dx%d\n", width, height, w, h);
			width = w;
			height = h;
			if (bits)
				free( bits);
			bits = malloc( width * height * sizeof(pixel_t));
			continue;
		}
		printf( "received frame %d, dims=%dx%d\n", f, width, height);
#if 1
		int i, j;
		for (j = 0; j < height; j++)
		{
			for (i = 0; i < width; i++)
			{
				printf( " %02X:%02X:%02X", bits[j * width + i].r, bits[j * width + i].g, bits[j * width + i].b);
			}
			printf( "\n");
		}
#endif
//		break;
	}
	return 0;
}
