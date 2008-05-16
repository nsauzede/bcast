TARGET= srv cli

CFLAGS=-Wall -Werror
CFLAGS+=-g -O0 -fno-omit-frame-pointer

LDFLAGS+=-luser32 -lgdi32 -lws2_32

all:	$(TARGET)

srv:	srv.o
	$(CC) -o $@ $^ $(LDFLAGS)

cli:	cli.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

