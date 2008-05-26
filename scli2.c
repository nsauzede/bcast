#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_thread.h>

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
		
		printf( "about to recv packet..\n");
		n = recv( sock, (void *)&packet, sizeof(packet), 0);
		if (n == SOCKET_ERROR)
		{
			
#ifdef _WIN32
			printf( "recv: error: %d\n", WSAGetLastError());
#endif
			printf( "sleeping..\n");
			SDL_Delay( 1000);
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
#if 0
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

pixel_t *bits = NULL;
int width = 1;
int height = 1;
int res_changed = 0;
SDL_mutex *mutex = NULL;

#define LOCK() SDL_mutexP( mutex)
#define UNLOCK() SDL_mutexV( mutex)

int sdl_init()
{
	SDL_Init( SDL_INIT_VIDEO);
	atexit( SDL_Quit);

	mutex = SDL_CreateMutex();

	return 0;
}

SOCKET sock;

int sock_thread( void *opaque)
{
	pixel_t *_bits = NULL;
	int _width = width;
	int _height = height;
	int _res_changed = 0;

	int done = 0;
	while (!done)
	{
		int n;
		int w, h, f;

		if (!_bits)
		{
			_bits = malloc( _width * _height * sizeof(pixel_t));
		}
		n = recv_bits( sock, _bits, _width, _height, &w, &h, &f);
#if 0
		int i, j;
		for (j = 0; j < _height; j++)
		{
			for (i = 0; i < _width; i++)
			{
				printf( " %02X:%02X:%02X", _bits[j * _width + i].r, _bits[j * _width + i].g, _bits[j * _width + i].b);
			}
			printf( "\n");
		}
#endif
#if 1
LOCK();
		if (n == -1)
		{
			break;
		}
		if (n == 1)
		{
			printf( "changing dims from %dx%d to %dx%d\n", _width, _height, w, h);
			_width = width = w;
			_height = height = h;
			_res_changed = 1;
			if (_bits)
			{
				free( _bits);
				_bits = NULL;
			}
#if 1
			if (bits)
			{
				free( bits);
				bits = NULL;
			}
			bits = malloc( _width * _height * sizeof(pixel_t));
			res_changed = 1;
#endif
		}
		else
		{
//			printf( "received frame %d, dims=%dx%d\n", f, _width, _height);
#if 1
			if (_bits && bits)
			{
				memcpy( bits, _bits, _width * _height * sizeof(pixel_t));
			}
#endif
		}
UNLOCK();
#endif
	}
	return 0;
}

int init_sock( int port, char *addr)
{
	printf( "creating sock..\n");
	sock = create_sock( port, addr);
	SDL_CreateThread( sock_thread, NULL);

	return 0;
}

int main( int argc, char* argv[])
{
#define PORT 12345
#if 1
#define ADDR "127.0.0.1"
#else
#define ADDR "192.168.0.10"
#endif
	int port = PORT;
	char *addr = ADDR;
	int arg = 1;

	if (argc > arg)
	{
		addr = argv[arg++];
	}
	printf( "initing sdl..\n");
	sdl_init();
	printf( "initing sock..\n");
	init_sock( port, addr);

	SDL_Surface *screen = NULL;
#define MIN_W 100
#define MIN_H 100
	int bpp = 32;
	int flags = 0;

	int done = 0;
	while (!done)
	{
LOCK();
		if (res_changed)
		{
			if (screen)
			{
				SDL_FreeSurface( screen);
				screen = 0;
			}
			res_changed = 0;
		}
		if (!screen)
		{
			int w, h;
			w = width;
			h = height;
			if (w < MIN_W)
				w = MIN_W;
			if (h < MIN_H)
				h = MIN_H;
			screen = SDL_SetVideoMode( w, h, bpp, flags);
		}
		if (screen)
		{
			SDL_FillRect( screen, 0, SDL_MapRGB(screen->format,128,0,0));
			if (bits)
			{
				int i, j;
				for (j = 0; j < height; j++)
				{
					for (i = 0; i < width; i++)
					{
						Uint32 col = SDL_MapRGB( screen->format, bits[j * width + i].r, bits[j * width + i].g, bits[j * width + i].b);
						SDL_Rect rect;
						rect.x = i;
						rect.y = j;
						rect.w = 1;
						rect.h = 1;
						SDL_FillRect( screen, &rect, col);
					}
				}
			}
			SDL_UpdateRect( screen, 0, 0, 0, 0);
		}
UNLOCK();
		SDL_Event event;
		while (SDL_PollEvent( &event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				case SDL_KEYDOWN:
					done = 1;
					break;
				default:
					break;
			}
			if (done)
				break;
		}
		SDL_Delay( 20);
	}

	return 0;
}

