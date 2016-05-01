/**
 * Serveur TFTP
 * Auteur : Mathieu Brochard
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tftp.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
       
#include <sys/types.h>
#include <sys/stat.h>

#define SOCK_BIND 0
#define SOCK_NO_BIND 1

struct t_rrq {
  SocketUDP *sock;
  AdresseInternet *addr;
  char *filename;
  size_t blksize;
  size_t windowsize;
};

// Créé et attache la socket
int initSocket(SocketUDP *sock, int bind);
// Quite le programme proprement
void quit(int code);
// Boucle principale
void *process_RRQ(void *arg);
// Attend une requête RRQ
void handle_RRQ(void);
// Ferme le programme lors de l'arrivée d'un signal
void handle_sig(int sig);

int port;
SocketUDP *sock = NULL;

int main(int argc, char **argv) {
  // Vérifie les argument
  if (argc != 2) {
    fprintf(stderr, "%s : port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  // Récuperation des arguments
  port = atoi(argv[1]);
    
  // Initialise la socket
  sock = malloc(sizeof(SocketUDP));
  if (initSocket(sock, SOCK_BIND) != 0) {
    quit(EXIT_FAILURE);
  }
  
  // Mise en place des signaux
  struct sigaction sa;
  sa.sa_handler = handle_sig;
  if (sigfillset(&sa.sa_mask) != 0) {
    perror("sigfillset");
    quit(EXIT_FAILURE);
  }
  if (sigaction(SIGINT, &sa, NULL) != 0) {
    perror("sigaction");
    quit(EXIT_FAILURE);
  }
  if (sigaction(SIGQUIT, &sa, NULL) != 0) {
    perror("sigaction");
    quit(EXIT_FAILURE);
  }
  
  // Attend une requête RRQ
  while (1) {
    handle_RRQ();
  }
  
  quit(EXIT_SUCCESS);
}

void handle_sig(int sig) {
  if (sig == SIGINT || sig == SIGQUIT) {
    quit(EXIT_SUCCESS);
  }
}

void handle_RRQ(void) {
  // Attend une requête RRQ
  size_t rrq_len = 512;
  char rrq_buf[rrq_len];
  AdresseInternet addr_cli;
  
  if (recvFromSocketUDP(sock, rrq_buf, rrq_len, &addr_cli, -1) == -1) {
    fprintf(stderr, "recvFromSocketUDP : impossible de recevoir le paquet.\n");
    return;
  }
  
  printf("Received packet -->\n");
  tftp_print(rrq_buf);
  
  // Si le paquet n'est pas de type RRQ, envoie une erreur
  uint16_t opcode;
  memcpy(&opcode, rrq_buf, sizeof(uint16_t));
  opcode = ntohs(opcode);
  
  if (opcode != RRQ) {
    tftp_send_error(sock, &addr_cli, ILLEG, "Le serveur attend un paquet RRQ.");
    return;
  }
  
  struct t_rrq rrq;
  rrq.blksize = (size_t) 0;
  rrq.windowsize = (size_t) 0;
  
  // Récupère les options de la requête
  size_t errcode;
  
  if ((errcode = tftp_get_opt(rrq_buf, &rrq.blksize, &rrq.windowsize)) != 0) {
    fprintf(stderr, "tftp_get_opt: %s\n", tftp_strerror(errcode));
    return;
  }
  
  // Lance le traitement du paquet RRQ dans un thread
  rrq.sock = sock;
  rrq.addr = &addr_cli;
  rrq.filename = malloc(sizeof(char) * strlen(rrq_buf + sizeof(uint16_t)) + 1);
  strncpy(rrq.filename, rrq_buf + sizeof(uint16_t), strlen(rrq_buf + sizeof(uint16_t)) + 1);
  pthread_t thread;
  if (pthread_create(&thread, NULL, process_RRQ, &rrq) != 0) {
    perror("pthread_create");
    return;
  }
}

int initSocket(SocketUDP *sock, int bind) {
  // Créé la socket
  if (initSocketUDP(sock) != 0) {
    fprintf(stderr, "initSocketUDP : impossible de créer la socket.\n");
    return -1;
  }
  
  // Attache la socket
  if (bind == SOCK_BIND) {
    if (attacherSocketUDP(sock, NULL, port, 0) != 0) {
      fprintf(stderr, "attacherSocketUDP : impossible d'attacher la socket.\n");
      return -1;
    }
  }
  
  return 0;
}

void quit(int code) {
  if (sock != NULL) {
      if (closeSocketUDP(sock) == -1) {
        fprintf(stderr, "closeSocketUDP : impossible de fermer la socket du serveur.\n");
        exit(EXIT_FAILURE);
      }
      free(sock);
  }
  
  exit(code);
}

void *process_RRQ(void *arg) {
  struct t_rrq *rrq = (struct t_rrq *) arg;
  SocketUDP *sock = rrq->sock;
  SocketUDP *sock_cli;
  AdresseInternet *addr_cli = rrq->addr;
  char *filename = rrq->filename;
  size_t blksize = rrq->blksize;
  size_t windowsize = rrq->windowsize;
  
  // Initialise une nouvelle socket dédiée au client
  sock_cli = malloc(sizeof(SocketUDP));
  if (initSocket(sock_cli, SOCK_NO_BIND) != 0) {
    tftp_send_error(sock, addr_cli, UNDEF, "Une erreur de connexion est survenue.");
    return NULL;
  }
    
  // Si des options ont été données, construit envoie un paquet OACK
  if (blksize != (size_t) 0 || windowsize != (size_t) 0) {
    size_t oackbuf_len = (size_t) 512;
    char oackbuf[oackbuf_len];
    size_t errcode;
    
    if ((errcode = tftp_make_OACK(oackbuf, &oackbuf_len, blksize, windowsize)) != 0) {
      fprintf(stderr, "tftp_make_OACK: %s\n", tftp_strerror(errcode));
      return NULL;
    }
    
    printf("Paquet sent -->\n");
    tftp_print(oackbuf);
    
    if ((errcode = tftp_send_OACK(sock_cli, addr_cli, oackbuf, oackbuf_len)) != 0) {
      fprintf(stderr, "tftp_send_OACK: %s\n", tftp_strerror(errcode));
      return NULL;
    }
  } else {
    blksize = (size_t) 512;
    windowsize = (size_t) 1;
  }
  
  // Ouvre le fichier demandé, envoie une erreur si l'ouvertue échoue
  int fd = open(filename, O_RDONLY);

  if (fd == -1) {
    perror("fd");
    if (errno == EACCES) {
      tftp_send_error(sock, addr_cli, FILNF, "Le fichier demandé est introuvable.");
    }
    return NULL;
  }

  if (fd != -1) {
    // Lit le fichier
    char fcontent_buf[blksize];
    size_t count;
    uint16_t block = 1;
    
    while ((count = read(fd, fcontent_buf, sizeof(fcontent_buf))) > 0) {
      // Construit le paquet DATA
      size_t data_len = blksize + sizeof(uint16_t) * 2;
      char data_buf[data_len];
      ssize_t errcode;
      
      if ((errcode = tftp_make_data(data_buf, &data_len, block, fcontent_buf, count)) != 0){
        fprintf(stderr, "tftp_make_data : %s\n", tftp_strerror(errcode));
        tftp_send_error(sock_cli, addr_cli, UNDEF, "Une erreur est survenue.");
        break;
      }
      
      printf("Paquet sent -->\n");
      tftp_print(data_buf);
      
      // Envoie le paquet DATA et attend le paquet ACK
      if ((errcode = tftp_send_DATA_wait_ACK(sock_cli, addr_cli, data_buf, data_len)) != 0) {
        fprintf(stderr, "tftp_send_DATA_wait_ACK : %s\n", tftp_strerror(errcode));
        tftp_send_error(sock_cli, addr_cli, UNDEF, "Une erreur est survenue.");
        break;
      }
      
      printf("Paquet reçu -->\n");
      printf("ACK - %d\n", block);
      
      block++;
    }
    
    // Ferme la socket dédiée au client
    if (closeSocketUDP(sock_cli) != 0) {
      fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
    }
    
    sock_cli = NULL;
          
    // Ferme le fichier
    if (close(fd) != 0) {
      perror("close");
    }
  }
  
  return NULL;
}
