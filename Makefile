CFLAGS = -std=c11 -Wall -Werror -Wextra -pedantic -D_XOPEN_SOURCE=600 -I./include
LDFLAGS = -lpthread

all: bin/server bin/client

bin/server: src/server.c obj/AdresseInternet.o obj/SocketUDP.o obj/tftp.o obj/config.o
	mkdir -p bin/
	$(CC) $(CFLAGS) $< -o $@ obj/AdresseInternet.o obj/SocketUDP.o obj/tftp.o obj/config.o $(LDFLAGS)

bin/client: src/client.c obj/AdresseInternet.o obj/SocketUDP.o obj/tftp.o obj/config.o
	mkdir -p bin/
	$(CC) $(CFLAGS) $< -o $@ obj/AdresseInternet.o obj/SocketUDP.o obj/tftp.o obj/config.o

obj/AdresseInternet.o: src/AdresseInternet.c
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@

obj/SocketUDP.o: src/SocketUDP.c
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@

obj/tftp.o: src/tftp.c
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@
	
obj/config.o: src/config.c
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r obj/

distclean:
	$(RM) -r obj/ bin/
