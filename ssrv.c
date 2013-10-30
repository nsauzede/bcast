#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <SDL.h>

#include "common.h"

#include <SDL_syswm.h>

#ifndef _WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

int get_bits( int x, int y, int w, int h, pixel_t *dest)
{
	int result = 0;

	if (!dest)
		return -1;
#ifdef _WIN32
	HWND hwnd;
#if 0
	POINT p;
	p.x = x;
	p.y = y;
	hwnd = WindowFromPoint( p);
#else
	hwnd = GetDesktopWindow();
#endif
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
//			hdc = GetDC( hwnd);
			hdc = GetWindowDC( hwnd);
			if (hdc == NULL)
			{
				printf( "%s: Failed to get HDC of HWND=%p\n", __func__, hwnd);
			}
			else
			{
//				printf( "%s: got HDC=%p\n", __func__, hdc);
//				x = 0;				y = 0;
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
#elif 1
        char* display = getenv( "DISPLAY");
	Display* pDisplay = XOpenDisplay( display);
	do
	{
		XSync( pDisplay, False);
	} while (XPending( pDisplay));
	Drawable hWindow;
	hWindow = XDefaultRootWindow( pDisplay);
	XImage* pImage;
	unsigned long planes = (typeof(planes))-1;
	int format = ZPixmap;
	pImage = XGetImage( pDisplay, hWindow, x, y, w, h, planes, format);
	int i, j;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			unsigned long pixel;
			pixel = XGetPixel( pImage, i, j);
#if 0
			dest->b = pixel & 0xFF;
			dest->g = (pixel & 0xFF00) >> 8;
			dest->r = (pixel & 0xFF0000) >> 16;
#else
			dest->r = pixel & 0xFF;
			dest->g = (pixel & 0xFF00) >> 8;
			dest->b = (pixel & 0xFF0000) >> 16;
#endif
			dest++;
		}
	}
	XDestroyImage( pImage);
	XCloseDisplay( pDisplay);
#else
	int i, j;
	static int f = 0;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			dest->r = f+1*j+2*i;
			dest->g = f+3*j+4*(i+1);
			dest->b = f+5*j+6*(i+2);
			dest++;
		}
	}
	f+=10;
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
		send( sock, (const void *)&packet, sizeof( packet), 0);
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
	if (s == -1)
	{
		perror( "socket");
		exit( 1);
	}
	int on = 1;
	if (setsockopt( s, SOL_SOCKET, SO_BROADCAST, (void *)&on, sizeof( on)) == -1)
	{
		perror( "setsockopt");
		exit( 2);
	}
	struct sockaddr_in sa;
	memset( &sa, 0, sizeof( sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons( port);
	sa.sin_addr.s_addr = inet_addr( addr);
//	sa.sin_addr.s_addr = INADDR_ANY;
#if 1
	if (connect( s, (struct sockaddr *)&sa, sizeof( sa)) == -1)
	{
		perror( "connect");
		exit( 3);
	}
#else
	if (bind( s, (struct sockaddr *)&sa, sizeof( sa)) == -1)
	{
		perror( "bind");
		exit( 3);
	}
#endif

	return s;
}

int main( int argc, char *argv[])
{
#define W 40
#define H 40
#define X 100
#define Y 100
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
	int arg = 1;

	if (argc > arg)
	{
		addr = argv[arg++];
	}
	sock = create_sock( port, addr);
	size = sizeof(*dest) * w * h;
	dest = malloc( sizeof( *dest) * w * h);
	int frame = 0;
	SDL_Init( SDL_INIT_VIDEO);
	atexit( SDL_Quit);
	int bpp = 16;
	int flags = 0;
//	flags |= SDL_SRCCOLORKEY;
	flags |= SDL_RESIZABLE;
	SDL_Surface *screen = 0;
	int dirty = 1;
	int end = 0;
	int old_w = -1;
	int old_h = -1;
	while (!end)
	{
		if (screen)
		{
#if 1
			SDL_SysWMinfo info;
			SDL_VERSION(&info.version);
			if ( SDL_GetWMInfo(&info) )
			{
#ifdef _WIN32
				HWND hwnd = info.window;
				printf( "got HWND=%08lx\n", hwnd);
				RECT rect;
				if (!GetWindowRect( hwnd, &rect))
				{
					printf( "%s: Failed to get rect of HWND=%p\n", __func__, hwnd);
				}
				else
				{
					printf( "%s: got rect=(%ld;%ld;%ld;%ld)\n", __func__, rect.left, rect.right, rect.top, rect.bottom);
				}
#endif
			}
#endif
		}
		get_bits( x, y, w, h, dest);
#if 0
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
		SDL_Event event;
		while (!end && SDL_PollEvent( &event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					end = 1;
					break;
				case SDL_KEYDOWN:
					end = 1;
					break;
				case SDL_VIDEORESIZE:
					w = event.resize.w;
					h = event.resize.h;
					break;
				default:
					break;
			}
		}
		if ((old_w != w) || (old_h != h))
		{
			old_w = w;
			old_h = h;
			if (screen)
			{
				SDL_FreeSurface( screen);
				screen = 0;
			}
			screen = SDL_SetVideoMode( w, h, bpp, flags);;
			dirty = 1;
		}
		if (dirty && screen)
		{
			dirty = 0;
			SDL_Rect rect;
			Uint32 col;
			rect.x = 0;
			rect.y = 0;
			rect.w = w - 1;
			rect.h = h - 1;
			col = SDL_MapRGB( screen->format, 58, 64, 255);
			SDL_FillRect( screen, &rect, col);
			SDL_UpdateRect( screen, 0, 0, 0, 0);
		}
		else
			SDL_Delay( 100);
	}

	return 0;
}
