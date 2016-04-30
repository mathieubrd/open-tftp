#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tftp.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>

// Récupère les arguments
// Retourne -1 si les arguments donnés sont invalides
int parse_args(int argc, char **argv);
// Affiche l'usage du programme
void usage(char *progname);
// Initialise la socket
int initSocket(void);
// Quitte le programme proprement
void quit(int code);
// Démarre le transfert
void run(void);
// Quitte le programme proprement à la réception d'un signal
void handle_sig(int sig);

SocketUDP sock;
char *ip, *filename, *destfile = NULL;
int *port = NULL;
size_t blksize, windowsize = (size_t) 0;
AdresseInternet *dst = NULL;

int main(int argc, char **argv) {  
  // Récupère les arguments
  if (parse_args(argc, argv) != 0) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  
  // Initialise la socket
  if (initSocket() != 0) {
    quit(EXIT_FAILURE);
  }
  
  // Configuration des signaux
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
  
  // Démarre le transfert
  run();
  
  return EXIT_SUCCESS;
}

int parse_args(int argc, char **argv) {
    // Sur GNU, getopt ne s'arrête pas s'il rencontre un argument qui n'est pas
    // accepté dans optstring.
    // Sur un système POSIX non GNU (Mac OS X), getopt s'arrête s'il rencontre
    // un argument qui n'est pas spécifié dans optstring.
    // On doit donc récuperer manuellement l'ip et le port, puis les options
    // et enfin, le fichier source et le fichier de destination.
    
    // Récupère l'adresse IP et le port
    ip = argv[1];
    if (ip == NULL) {
      return -1;
    }
    port = malloc(sizeof(int));
    *port = atoi(argv[2]);
    if (port == NULL) {
      return -1;
    }
    
    // Récupère les options
    int ch;
    optind += 2;
    while ((ch = getopt(argc, argv, "b:w:")) != -1) {
      switch (ch) {
        case 'b':
          blksize = (size_t) atoi(optarg);
          break;
        case 'w':
          windowsize = (size_t) atoi(optarg);
          break;
        default:
          return -1;
      }
    }
    
    // Récupère le nom du fichier source et de destination
    filename = argv[optind];
    if (filename == NULL) {
      return -1;
    }
    destfile = argv[optind + 1];
    if (destfile == NULL) {
      return -1;
    }
    
    return 0;
}

void usage(char *progname) {
    fprintf(stderr, "Usage: %s ip port [-b blksize] [-w windowsize] src dest\n", progname);
}

void handle_sig(int sig) {
  if (sig == SIGINT || sig == SIGQUIT) {
    quit(EXIT_SUCCESS);
  }
}

int initSocket(void) {
  // Créé la socket
  if (initSocketUDP(&sock) != 0) {
    fprintf(stderr, "initSocketUDP : impossible de créer la socket.\n");
    return -1;
  }
  dst = AdresseInternet_new(ip, (uint16_t) *port);
  if (dst == NULL) {
    fprintf(stderr, "AdresseInternet_new : impossible de créer une AdresseInternet.\n");
    return -1;
  }
   
  return 0;
}

void run(void) {
  AdresseInternet addrserv;
  size_t buffer_len = (size_t) 512;
  
  char buffer[buffer_len];
  ssize_t errcode;
  
  // Construit le paquet RRQ avec options, s'il y a.
  size_t rrqbuf_len = 512;
  char rrqbuf[rrqbuf_len];
  
  if ((errcode = tftp_make_rrq_opt(rrqbuf, &rrqbuf_len, filename, (size_t) blksize, (size_t) windowsize)) != 0) {
    fprintf(stderr, "tftp_make_rrq_opt: %s\n", tftp_strerror(errcode));
    exit(EXIT_FAILURE);
  }
  
  // Si des options ont été spécifiées, envoie le paquet RRQ et attend le paquet OACK puis attend le premier paquet DATA.
  // Sinon, envoie le paquet RRQ et attend le premier paquet DATA
  if (blksize != (size_t) 0 || windowsize != (size_t) 0) {
    if ((errcode = tftp_send_RRQ_wait_OACK(&sock, dst, &addrserv, rrqbuf, rrqbuf_len, buffer, &buffer_len)) != 0) {
      fprintf(stderr, "tftp_send_RRQ_wait_OACK: %s\n", tftp_strerror(errcode));
      exit(EXIT_FAILURE);
    }
    
    printf("%s\n", buffer + sizeof(uint16_t));
    
    // Récupère les options dans le paquet OACK
    if ((errcode = tftp_get_opt(buffer, &blksize, &windowsize)) != 0) {
      fprintf(stderr, "tftp_get_opt: %s\n", tftp_strerror(errcode));
      exit(EXIT_FAILURE);
    }
    
    // Attend le premier paquet DATA    
    if ((errcode = tftp_wait_DATA_with_timeout(&sock, &addrserv, buffer, &buffer_len)) != 0) {
      fprintf(stderr, "tftp_wait_DATA: %s\n", tftp_strerror(errcode));
      exit(EXIT_FAILURE);
    }
  } else {
    if ((errcode = tftp_send_RRQ_wait_DATA(&sock, dst, filename, &addrserv, buffer, &buffer_len)) != 0) {
      fprintf(stderr, "tftp_send_RRQ_wait_DATA: %s\n", tftp_strerror(errcode));
      quit(EXIT_FAILURE);
    }
  }

  uint16_t block = 0;
  memcpy(&block, buffer + sizeof(uint16_t), sizeof(uint16_t));
  block = ntohs(block);
  
  printf("block reçu : %d\n", block);
  
  // Ouvre le fichier
  int fd = open(destfile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("open");
    quit(EXIT_FAILURE);
  }
  
  if (write(fd, buffer + sizeof(uint16_t) * 2, buffer_len - sizeof(uint16_t) * 2) == -1) {
    perror("write");
    quit(EXIT_FAILURE);
  }
  
  while (buffer_len >= blksize - 4) {
    // Envoie le premier paquet ACK et attend le paquet DATA
    buffer_len = 512;
    if ((errcode = tftp_make_ack(buffer, &buffer_len, block)) != 0) {
      fprintf(stderr, "tftp_make_ack : %s\n", tftp_strerror(errcode));
      quit(EXIT_FAILURE);
    }
    
    size_t data_len = blksize + 4;
    char data[data_len];
    
    if ((errcode = tftp_send_ACK_wait_DATA(&sock, &addrserv, buffer, buffer_len, data, &data_len)) != 0) {
      fprintf(stderr, "tftp_send_ACK_wait_DATA : %s\n", tftp_strerror(errcode));
      quit(EXIT_FAILURE);
    }
    memcpy(&block, data + sizeof(uint16_t), sizeof(uint16_t));
    block = ntohs(block);
    
    printf("block reçu : %d\n", block);
    
    if (write(fd, data + sizeof(uint16_t) * 2, data_len - sizeof(uint16_t) * 2) == -1) {
      perror("write");
      quit(EXIT_FAILURE);
    }
    
    if (data_len < blksize + 4) {
      break;
    }
  }
  
  // Envoie le dernier paquet ACK
  buffer_len = 512;
  char ackbuf[buffer_len];
  
  if ((errcode = tftp_make_ack(ackbuf, &buffer_len, block)) != 0) {
    fprintf(stderr, "tftp_make_ack : %s\n", tftp_strerror(errcode));
    quit(EXIT_FAILURE);
  }
  
  if ((errcode = tftp_send_last_ACK(&sock, &addrserv, ackbuf, buffer_len)) != 0) {
    fprintf(stderr, "tftp_send_last_ACK : %s\n", tftp_strerror(errcode));
    quit(EXIT_FAILURE);
  }
}

void quit(int code) {
  if (closeSocketUDP(&sock) != 0) {
    fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
    exit(EXIT_FAILURE);
  }
  
  exit(code);
}
