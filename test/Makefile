CFLAGS = -std=c11 -Wall -Werror -Wextra -pedantic -D_XOPEN_SOURCE=600 -I../include

all: testrrq

testrrq: testrrq.c
	cd .. && make clean && make
	$(CC) $(CFLAGS) $< -o $@ ../obj/tftp.o ../obj/SocketUDP.o ../obj/AdresseInternet.o
	
clean:
	$(RM) *.o
	
distclean:
	$(RM) *.o testrrq
