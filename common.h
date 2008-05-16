#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifndef _WIN32
typedef int SOCKET;
#endif

#define PAYLOAD_SIZE	15

typedef struct pixel {
	int r, g, b;
} pixel_t;

typedef struct packet {
	uint16_t frame;
	uint16_t seq;	// 0==new frame start
	uint16_t width;
	uint16_t height;
	unsigned char payload[PAYLOAD_SIZE];
} packet_t;
