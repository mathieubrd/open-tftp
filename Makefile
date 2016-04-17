CFLAGS = -std=c11 -Wall -Werror -Wextra -pedantic -D_XOPEN_SOURCE=500 -I./include
LDLIBS = -L./lib64
LDFLAGS = -lSocketUDP -lAdresseInternet -lpthread

all: obj/tftp.o bin/server bin/client

obj/tftp.o: src/tftp.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c $^ -o $@

bin/server: src/server.c obj/tftp.o
	mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) $(LDFLAGS)

bin/client: src/client.c obj/tftp.o
	mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) $(LDFLAGS)

clean:
	$(RM) -r obj

distclean:
	$(RM) -r obj bin
