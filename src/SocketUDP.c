#include <SocketUDP.h>

int initSocketUDP(SocketUDP *sock) {
  sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock->sockfd == -1) {
    return -1;
  }
  sock->bound = -1;
  
  return 0;
}

int attacherSocketUDP(SocketUDP *sock, const char *adresse, uint16_t port,
  int flags) {
  
  if (adresse != NULL) {
    sock->addr = AdresseInternet_new(adresse, port);
    if (sock->addr == NULL) {
      return -1;
    }
  } else if (adresse == NULL) {
    if (flags == 0) {
      sock->addr = AdresseInternet_any(port);
    } else if (flags == LOOPBACK) {
      sock->addr = AdresseInternet_loopback(port);
    }
  }
  
  struct sockaddr sockaddr;
  if (AdresseInternet_to_sockaddr(sock->addr, &sockaddr) != 0) {
    return -1;
  }
  socklen_t sockaddr_len = sizeof(sockaddr);
  
  if (bind(sock->sockfd, &sockaddr, sockaddr_len) != 0) {
    perror("bind");
    return -1;
  }
  
  sock->bound = 0;
  
  return 0;
}

int estAttachee(SocketUDP *sock) {
  return sock->bound;
}

int getLocalName(SocketUDP *sock, char *buffer, int taille) {
  char name[taille];
  char ip[1];
  if (AdresseInternet_getinfo(sock->addr, name, taille, ip, 0) != 0) {
    return -1;
  }
  strncpy(buffer, name, (size_t) taille);
  
  return strlen(buffer);
}

int getLocalIP(SocketUDP *sock, char *buffer, int taille) {
  char ip[16];
  if (AdresseInternet_getIP(sock->addr, ip, sizeof(ip)) != 0) {
    return -1;
  }
  strncpy(buffer, ip, taille);
  
  return strlen(buffer);
}

ssize_t writeToSocketUDP(SocketUDP *sock, const AdresseInternet *adresse,
  const char *buffer, int length) {
  
  if (sock == NULL || adresse == NULL || buffer == NULL) {
    return -1;
  }
  
  struct sockaddr_storage sockaddr;
  if (AdresseInternet_to_sockaddr(adresse, (struct sockaddr *) &sockaddr) != 0) {
    return -1;
  }
  socklen_t sockaddr_len = sizeof(sockaddr);
  
  ssize_t count = sendto(sock->sockfd, buffer, (size_t) length, 0, (struct sockaddr *) &sockaddr,
    sockaddr_len);
    
  if (count == -1) {
    perror("sendto");
  }
  
  return count;
}

ssize_t recvFromSocketUDP(SocketUDP *sock, char *buffer, int length,
  AdresseInternet *adresse, int timeout) {
  
  if (sock == NULL || buffer == NULL) {
    return -1;
  }
  
  struct sockaddr sockaddr;
  memset(&sockaddr, 0, sizeof(sockaddr));
  socklen_t sockaddr_len = sizeof(sockaddr);
  
  struct sigaction act;
  act.sa_handler = handleAlarm;
  act.sa_flags = 0;
  
  if (sigemptyset(&act.sa_mask) != 0) {
    return -1;
  }
  
  if (sigaction(SIGALRM, &act, NULL) != 0) {
    return -1;
  }
  
  alarm(timeout);
  
  ssize_t count = recvfrom(sock->sockfd, buffer, length, 0, &sockaddr,
    &sockaddr_len);
    
  if (adresse != NULL) {
    sockaddr_to_AdresseInternet(&sockaddr, adresse);
  }
  
  return count;
}

int closeSocketUDP(SocketUDP *sock) {
  if (close(sock->sockfd) != 0) {
    return -1;
  }
  
  return 0;
}

void handleAlarm(int sig) {
  if (sig == SIGALRM) {
    // Timeout, nothing to do
  }
}
