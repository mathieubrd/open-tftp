CFLAGS = -std=c11 -Wall -Werror -Wextra -pedantic -D_XOPEN_SOURCE=600 -I../include
LDFLAGS = -lpthread

all: printer

printer: printer.c ../obj/AdresseInternet.o ../obj/SocketUDP.o ../obj/tftp.o
	$(CC) $(CFLAGS) $< -o $@ ../obj/AdresseInternet.o ../obj/SocketUDP.o ../obj/tftp.o $(LDFLAGS)

../obj/AdresseInternet.o: ../src/AdresseInternet.c
	cd ../
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@

../obj/SocketUDP.o: ../src/SocketUDP.c
	cd ../
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@

../obj/tftp.o: ../src/tftp.c
	cd ../
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $< -o $@

distclean:
	$(RM) printer
