TARGET= srv cli

UNAME=$(shell uname)

ifeq ($(UNAME),mingw)
WIN32=1
endif

CFLAGS=-Wall -Werror
CFLAGS+=-g -O0 -fno-omit-frame-pointer

ifdef WIN32
LDFLAGS+=-luser32 -lgdi32 -lws2_32
endif

all:	$(TARGET)

srv:	srv.o
	$(CC) -o $@ $^ $(LDFLAGS)

cli:	cli.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

