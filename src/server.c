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
  char *buffer;
};

// Créé et attache la socket
int initSocket(SocketUDP *sock, int bind);
// Quite le programme proprement
void quit(int code);
// Boucle principale
void *process_RRQ(void *arg);
// Attend une requête RRQ
void handle_RRQ(void);
// Ferme le programme lors de l'arrivée d'un signal SIGINT
void handle_sig(int sig);

SocketUDP sock;

int main(void) {
  // Initialise la socket
  if (initSocket(&sock, SOCK_BIND) != 0) {
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
  if (sig == SIGINT || sig == SIGQUIT)
    quit(EXIT_SUCCESS);
  }
}

void handle_RRQ(void) {
  // Attend une requête RRQ
  size_t rrq_len = 512;
  char rrq_buf[rrq_len];
  AdresseInternet addr_cli;
  
  if (recvFromSocketUDP(&sock, rrq_buf, rrq_len, &addr_cli, -1) == -1) {
    fprintf(stderr, "recvFromSocketUDP : impossible de recevoir le paquet.\n");
    return;
  }
  
  // Si le paquet n'est pas de type RRQ, envoie une erreur
  uint16_t opcode;
  memcpy(&opcode, rrq_buf, sizeof(uint16_t));
  
  if (opcode != RRQ) {
    tftp_send_error(&sock, &addr_cli, ILLEG, "Le serveur attend un paquet RRQ.");
    return;
  }
  
  // Lance le traitement du paquet RRQ dans un thread
  struct t_rrq rrq;
  rrq.sock = &sock;
  rrq.addr = &addr_cli;
  rrq.buffer = rrq_buf;
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
    if (attacherSocketUDP(sock, NULL, 25565, 0) != 0) {
      fprintf(stderr, "attacherSocketUDP : impossible d'attacher la socket.\n");
      return -1;
    }
  }
  
  return 0;
}

void quit(int code) {
  if (closeSocketUDP(&sock) == -1) {
    fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
    exit(EXIT_FAILURE);
  }
  
  exit(code);
}

void *process_RRQ(void *arg) {
  struct t_rrq *rrq = (struct t_rrq *) arg;
  SocketUDP *sock = rrq->sock;
  AdresseInternet *addr_cli = rrq->addr;
  char *rrq_buf = rrq->buffer;
  
  while (1) {
    // Ouvre le fichier demandé, envoie une erreur si l'ouvertue échoue
    char *filename = &rrq_buf[sizeof(uint16_t)];
    int fd = open(filename, O_RDONLY);
    
    if (fd == -1) {
      perror("fd");
      if (errno == EACCES) {
	tftp_send_error(sock, addr_cli, FILNF, "Le fichier demandé est introuvable.");
      }
      break;
    }
    
    if (fd != -1) {
      // Initialise une nouvelle socket dédiée au client
      SocketUDP sock_cli;
      if (initSocket(&sock_cli, SOCK_NO_BIND) != 0) {
	tftp_send_error(sock, addr_cli, UNDEF, "Une erreur de connexion est survenue.");
	break;
      }
      
      // Lit le fichier
      char fcontent_buf[508];
      size_t count;
      uint16_t block = 1;
      
      while ((count = read(fd, fcontent_buf, sizeof(fcontent_buf))) > 0) {
	// Construit le paquet DATA
	size_t data_len = 512;
	char data_buf[data_len];
	
	if (tftp_make_data(data_buf, &data_len, block, fcontent_buf, count) != 0) {
	  tftp_send_error(&sock_cli, addr_cli, UNDEF, "Une erreur est survenue.");
	  break;
	}
	
	// Envoie le paquet DATA et attend le paquet ACK
	if (tftp_send_DATA_wait_ACK(&sock_cli, addr_cli, data_buf, data_len) != 0) {
	  tftp_send_error(&sock_cli, addr_cli, UNDEF, "Une erreur est survenue.");
	  break;
	}
	
	block++;
      }
      
      // Ferme la socket dédiée au client
      if (closeSocketUDP(&sock_cli) != 0) {
	fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
      }
    }
    
    // Ferme le fichier
    if (close(fd) != 0) {
      perror("close");
    }
  }
  
  return NULL;
}
