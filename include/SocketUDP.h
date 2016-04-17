#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "AdresseInternet.h"
#include "AdresseInternetType.h"

#include <sys/types.h>
#include <sys/socket.h>

#define LOOPBACK 1

typedef struct SocketUDP {
  int sockfd;
  AdresseInternet *addr;
  int bound;
} SocketUDP;

int initSocketUDP(SocketUDP *);

int attacherSocketUDP(SocketUDP *, const char *, uint16_t,
  int);

int estAttachee(SocketUDP *);

int getLocalName(SocketUDP *, char *, int);

int getLocalIP(SocketUDP *, char *, int);

ssize_t writeToSocketUDP(SocketUDP *, const AdresseInternet *, const char *,
  int);
  
ssize_t recvFromSocketUDP(SocketUDP *, char *, int, AdresseInternet *, int);

int closeSocketUDP(SocketUDP *);

void handleAlarm(int);


#endif
