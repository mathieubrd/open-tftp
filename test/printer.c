#include <stdlib.h>
#include <stdio.h>

#include <tftp.h>

int main(void) {
  // Construit un paquet RRQ avec options
  size_t len = (size_t) 512;
  char buf[len];
  tftp_make_rrq_opt(buf, &len, "fichier", (size_t) 1024, (size_t) 0);
  
  // Affiche le paquet RRQ
  tftp_print(buf);
  
  // Construit un paquet ACK
  len = (size_t) 512;
  tftp_make_ack(buf, &len, (uint16_t) 541);
  
  // Affiche le paquet ACK
  tftp_print(buf);
  
  // Construit un paquet ERROR
  len = (size_t) 512;
  tftp_make_error(buf, &len, FILNF, errors[FILNF]);
  
  // Affiche le paquet ERROR
  tftp_print(buf);
  
  // Construit un paquet OACK
  len = (size_t) 512;
  tftp_make_OACK(buf, &len, (size_t) 1024, (size_t) 0);
  
  // Affiche le paquet OACK
  tftp_print_OACK(buf);
  
  return EXIT_SUCCESS;
}
