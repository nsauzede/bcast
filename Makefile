TARGET= srv.exe cli.exe scli.exe scli2.exe

UNAME=$(shell uname)

ifeq ($(UNAME),MINGW32_NT-5.1)
WIN32=1
endif

CFLAGS=-Wall -Werror
CFLAGS+=-g -O0 -fno-omit-frame-pointer

ifdef WIN32
LDFLAGS+=-luser32 -lgdi32 -lws2_32
else
SRV_LDFLAGS=-L/usr/X11R6/lib64 -L/usr/X11R6/lib -lX11
endif

SDL_CONFIG=sdl-config
SDL_CFLAGS=`$(SDL_CONFIG) --cflags`
SDL_LDFLAGS=`$(SDL_CONFIG) --libs`
ifdef WIN32
# remove -mwindows to get stdout output
SDL_LDFLAGS+=-mno-windows
endif


all:	$(TARGET)

srv.exe:	srv.o
	$(CC) -o $@ $^ $(LDFLAGS) $(SRV_LDFLAGS)

cli.exe:	cli.o
	$(CC) -o $@ $^ $(LDFLAGS)

scli.o:	CFLAGS+=$(SDL_CFLAGS)
scli.exe:	scli.o
	$(CC) -o $@ $^ $(LDFLAGS) $(SDL_LDFLAGS)

scli2.o:	CFLAGS+=$(SDL_CFLAGS)
scli2.exe:	scli2.o
	$(CC) -o $@ $^ $(LDFLAGS) $(SDL_LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

clobber: clean
	$(RM) *~

