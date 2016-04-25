CFLAGS = -std=c11 -Wall -Werror -Wextra -pedantic -D_XOPEN_SOURCE=600 -I../include

all: bin/testrrq

bin/testrrq: testrrq.c
	cd .. && make clean && make
	mkdir bin/
	$(CC) $(CFLAGS) $< -o $@ ../obj/tftp.o ../obj/SocketUDP.o ../obj/AdresseInternet.o
	
clean:
	$(RM) *.o

distclean:
	$(RM) -rf *.o bin/
