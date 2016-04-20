#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "AdresseInternet.h"

/** 
 * construit une adresse internet à partir d’un éventuel nom (sous
forme DNS ou IP) et d’un numéro de port. L’argument adresse est un
 * pointeur vers une variable de type AdresseInternet, qui est allouée et initialisée 
 * avec l’adresse internet construite. Valeur de retour : en cas
 * de succès, la fonction renvoie le pointeur. En cas d’erreur la fonction
 * renvoie NULL.
 */
AdresseInternet * _AdresseInternet_new(int flag, const char* nom, uint16_t port){
  // on va faire appel à getaddrinfo pour savoir si l'adresse existe.
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  // Si nom est NULL, avec AI_PASSIVE on récupère une adresse ANY qui
  // permettra de faire un bind sur toutes les interfaces réseau
  // avec AI_ADDRCONFIG on récupère une adresse utilisable par au
  // moins une interface réseau.
  // avec AI_CANONNAME on force la résolution DNS inverse
  // avec AI_NUMERICSERV on empêche la résolution de service.
  // avec AI_NUMERICHOST on empêche la résolution de nom, le nom passé
  // doit être numérique.
  hints.ai_flags = flag | AI_NUMERICSERV ;
  hints.ai_family = AF_UNSPEC;// peu importe ipv4 ou ipv6

  char sport[8];
  snprintf(sport,8,"%d", port);
  //printf("le port: %s\n",sport);
  struct addrinfo *result;
  int s = getaddrinfo(nom, sport, &hints, &result);
  if (s != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return NULL;
  }
  /* on prend le premier dans la liste des adresses retournées */
  if (result->ai_family != AF_INET && result->ai_family != AF_INET6){
    fprintf(stderr, "adresse invalide\n");
    return NULL;
  }
  //printf("la famille: %d (AF_INET %d, AF_INET6 %d)\n",result->ai_family, AF_INET, AF_INET6);
  /*if (*adresselen < _DNS_NAME_MAX_SIZE + sizeof(result->ai_addrlen)){
    fprintf(stderr, "buffer d'adresse trop petit\n");
    return -1;
  }*/
  /* par défaut on n'a pas fait de recherche de nom DNS canonique */
  AdresseInternet * adresse = malloc(sizeof(AdresseInternet));
  adresse->nom[0] = 0;
  adresse->service[0] = 0;
  memcpy(&(adresse->sockAddr), result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);
  return adresse;
}

AdresseInternet * AdresseInternet_new(const char* nom, uint16_t port){
  return _AdresseInternet_new(AI_PASSIVE, nom, port);
}
/**
 * Idem, mais construit une adresse internet correspondant à toutes
 * les interfaces réseau à partir d’un numéro de port.
 */
AdresseInternet * AdresseInternet_any(uint16_t port){
  return _AdresseInternet_new(AI_PASSIVE, NULL, port);
}
/**
 * Idem, mais construit une adresse internet correspondant à
 * l'interface loopback à partir d’un numéro de port.
 */
AdresseInternet * AdresseInternet_loopback(uint16_t port){
  return _AdresseInternet_new(0, NULL, port);
}

void AdresseInternet_free(AdresseInternet *adresse){
  free(adresse);
}
/**
 * extrait d’une adresse internet l’adresse IP et le port
 * correspondants. L’argument adresse pointe vers un buffer
 * contenant une adresse. L’argument nomDNS (resp. nomPort)
 * pointe vers un buffer alloué de taille au moins tailleDNS
 * (resp. taillePort) dans lequel la fonc- tion va écrire une chaîne
 * de caractère (terminant par un 0) contenant le nom (resp. le port)
 * associé à l’adresse fournie. Lorsque cela est possible, la
 * résolution de nom est faite.  Si nomDNS ou nomPort est NULL,
 * l’extraction correspondante ne sera pas effectuée. Les deux ne
 * peuvent pas être NULL en même temps.  Valeur de retour : rend 0 en
 * cas de succès, et -1 en cas d’erreur.
 */
int AdresseInternet_getinfo(AdresseInternet *adresse, char *nomDNS, int tailleDNS, char *nomPort, int taillePort){
  /*if (adresselen < _DNS_NAME_MAX_SIZE + sizeof(sa_family_t)){
    return -1;
  }*/
  char *host;
  char *serv;
  if (adresse->nom[0] != 0){ // on connait déjà le nom DNS
    strncpy(nomDNS, adresse->nom, tailleDNS);
    nomDNS[tailleDNS-1] = 0;
    host = NULL;
  } else {
    host = nomDNS;
  };
  if (adresse->service[0] != 0){
    strncpy(nomPort, adresse->service, taillePort);
    nomPort[taillePort-1] = 0;
    serv = NULL;
  } else {
    serv = nomPort;
  };
  //  printf("host %d, serv %d\n",host, serv);
  if ((serv == NULL) & (host == NULL)){
    return 0;
  };
  int n = getnameinfo((struct sockaddr *)&adresse->sockAddr, sizeof(adresse->sockAddr), host, tailleDNS, serv, taillePort, 0);
  if (n != 0){
    return -1;
  };  
  // ensuite on mémorise les valeurs trouvées.
  strncpy(adresse->nom, host, _DNS_NAME_MAX_SIZE);
  strncpy(adresse->service, serv, _SERVICE_NAME_MAX_SIZE);
  return 0;
}
/**
 * extrait le numéro de port d’une adresse internet. L'argument
 * adresse se comporte comme dans la fonction
 * précédente.  Valeur de retour : renvoie le port en cas de succès,
 * et 0 en cas d’erreur (par exemple si adresse n’est pas
 * initialisée).
*/
uint16_t AdresseInternet_getPort(const AdresseInternet *adresse){
  switch (AdresseInternet_getDomain(adresse)){
  case AF_INET: {
    struct sockaddr_in *addr = (struct sockaddr_in *) &adresse->sockAddr;
    return ntohs(addr->sin_port); // in_port_t
    break;
  };
  case AF_INET6: {
    struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &adresse->sockAddr;
    return ntohs(addr->sin6_port); // in6_port_t
    break;
  };
  }
  return 0;
}

int AdresseInternet_getIP(const AdresseInternet *adresse, char *IP, int tailleIP){
  switch (AdresseInternet_getDomain(adresse)){
  case AF_INET: {
    struct sockaddr_in *addr = (struct sockaddr_in *) &adresse->sockAddr;
    if (inet_ntop(AF_INET, &(addr->sin_addr.s_addr), IP, tailleIP) == NULL){
      return -1;
    }
    return 0;
    break;
  };
  case AF_INET6: {
    struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &adresse->sockAddr;
    if (inet_ntop(AF_INET6, addr->sin6_addr.s6_addr, IP, tailleIP) == NULL){
      return -1;
    }
    return 0;
    break;
  };
  }
  return 0;
}

int AdresseInternet_getDomain(const AdresseInternet *adresse){
  return  adresse->sockAddr.ss_family;
}
/** 
 * Construit une adresse internet à partir d'une structure sockaddr La
 * structure addresse doit être suffisamment grande pour pouvoir
 * accueillir l’adresse.  Valeur de retour : 0 en cas de succès, -1 en
 * cas d’échec.
 */
int sockaddr_to_AdresseInternet(const struct sockaddr *addr, AdresseInternet *adresse){
  // on a une sockaddr_storage:
  memcpy(&(adresse->sockAddr), addr, sizeof(*addr));
  adresse->nom[0] = 0;
  adresse->service[0] = 0;
  return 0;
}
/** 
 * Construit une structure sockaddr à partir d'une adresse
 * internet. La structure addr doit être suffisamment grande pour
 * pouvoir accueillir l’adresse.  Valeur de retour : 0 en cas de
 * succès, -1 en cas d’échec.
 */
int AdresseInternet_to_sockaddr(const AdresseInternet *adresse, struct sockaddr *addr){
  sa_family_t famille = adresse->sockAddr.ss_family;
  switch (famille){
  case AF_INET:
    memcpy(addr, &(adresse->sockAddr), sizeof(struct sockaddr_in));
    break;
  case AF_INET6:
    memcpy(addr, &(adresse->sockAddr), sizeof(struct sockaddr_in6));
    break;
  default:
    return -1;
  }
  return 0;
}
/**
 * compare deux adresse internet adresse1 et adresse2.  Valeur de
 * retour : rend 0 si les adresses sont différentes, 1 si elles sont
 * identiques (même IP et même port), et -1 en cas d’erreur.
 *
 */
int AdresseInternet_compare(const AdresseInternet *adresse11, const AdresseInternet *adresse22){
  char IP1[40];
  char IP2[40];
  if (AdresseInternet_getIP(adresse11, IP1, 40) == -1){
    return -1;
  }
  if (AdresseInternet_getIP(adresse22, IP2, 40) == -1){
    return -1;
  }
  if (strcmp(IP1, IP2) != 0){ // ce ne sont pas les mêmes
    return 0;
  }
  // on compare maintenant les ports
  uint16_t n1 = AdresseInternet_getPort(adresse11);
  if (n1 == 0){
    return -1;
  }
  uint16_t n2 = AdresseInternet_getPort(adresse22);
  if (n2 == 0){
    return -1;
  }
  if (n1 == n2){
    return 1;
  }
  return 0;
}

int AdresseInternet_copy(AdresseInternet *adrdst, const AdresseInternet *adrsrc){
  if (memcpy(adrdst, adrsrc, sizeof(AdresseInternet))==NULL){
    return -1;
  }
  return 0;
}

