#include <stdint.h>

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
#define SOCKET_ERROR -1
#endif

#define PAYLOAD_SIZE	15

#pragma pack(1)

typedef struct pixel {
	uint8_t r, g, b;
} pixel_t;

typedef struct packet {
	uint16_t frame;
	uint16_t seq;	// 0==new frame start
	uint16_t width;
	uint16_t height;
	unsigned char payload[PAYLOAD_SIZE];
} packet_t;

#pragma pack()
