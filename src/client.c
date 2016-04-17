#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tftp.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

void quit(int code);

SocketUDP sock;

int main(int argc, char **argv) {
  // Vérifie les arguments
  if (argc != 3) {
    fprintf(stderr, "%s : fichier destination\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  // Créé la socket
  if (initSocketUDP(&sock) != 0) {
    fprintf(stderr, "initSocketUDP : impossible de créer la socket.\n");
    quit(EXIT_FAILURE);
  }
  AdresseInternet *dst = AdresseInternet_new("localhost", 25565);
  if (dst == NULL) {
    fprintf(stderr, "AdresseInternet_new : impossible de créer une AdresseInternet.\n");
    quit(EXIT_FAILURE);
  }
  
  // Envoie un paquet RRQ et attend le premier paquet DATA
  char *filename = argv[1];
  printf("%s\n", filename);
  AdresseInternet addrserv;
  size_t buffer_len = 512;
  char buffer[buffer_len];
  if (tftp_send_RRQ_wait_DATA_with_timeout(&sock, dst, filename, &addrserv, buffer, &buffer_len) != 0) {
    fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout: erreur\n");
    quit(EXIT_FAILURE);
  }

  uint16_t block;
  memcpy(&block, buffer + sizeof(uint16_t), sizeof(uint16_t));
  
  printf("block reçu : %d\n", block);
  
  // Ouvre le fichier
  char *destfile = argv[2];
  int fd = open(destfile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("open");
    quit(EXIT_FAILURE);
  }
  
  if (write(fd, buffer + sizeof(uint16_t) * 2, buffer_len - sizeof(uint16_t) * 2) == -1) {
    perror("write");
    quit(EXIT_FAILURE);
  }
  
  while (1) {
    // Envoie le premier paquet ACK et attend le paquet DATA
    buffer_len = 512;
    if (tftp_make_ack(buffer, &buffer_len, block) != 0) {
      fprintf(stderr, "tftp_make_ack : erreur\n");
      quit(EXIT_FAILURE);
    }
    size_t data_len = 512;
    char data[data_len];
    if (tftp_send_ACK_wait_DATA(&sock, &addrserv, buffer, buffer_len, data, &data_len) != 0) {
      fprintf(stderr, "tftp_send_ACK_wait_DATA : erreur\n");
      quit(EXIT_FAILURE);
    }
    memcpy(&block, data + sizeof(uint16_t), sizeof(uint16_t));
    
    printf("block reçu : %d\n", block);
    
    if (write(fd, data + sizeof(uint16_t) * 2, data_len - sizeof(uint16_t) * 2) == -1) {
      perror("write");
      quit(EXIT_FAILURE);
    }
    
    // Casse la boucle si la taille du dernier paquet reçu est inférieure à 512
    if (data_len < 512) {
      break;
    }
  }
  
  // Envoie le dernier paquet ACK
  buffer_len = 512;
  if (tftp_make_ack(buffer, &buffer_len, block) != 0) {
    fprintf(stderr, "tftp_make_ack : erreur\n");
    quit(EXIT_FAILURE);
  }
  if (tftp_send_last_ACK(&sock, &addrserv, buffer, buffer_len) != 0) {
    fprintf(stderr, "tftp_send_last : erreur\n");
    quit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}

void quit(int code) {
  if (closeSocketUDP(&sock) != 0) {
    fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
    exit(EXIT_FAILURE);
  }
  
  exit(code);
}
