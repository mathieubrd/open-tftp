#ifndef ADRESSEINTERNETTYPE_H_
#define ADRESSEINTERNETTYPE_H_

#define _DNS_NAME_MAX_SIZE 256
#define _SERVICE_NAME_MAX_SIZE 20

typedef struct {
  struct sockaddr_storage sockAddr;
  char nom[_DNS_NAME_MAX_SIZE];
  char service[_SERVICE_NAME_MAX_SIZE];
} _adresseInternet_struct;

typedef _adresseInternet_struct AdresseInternet;

#endif // ADRESSEINTERNETTYPE_H_
