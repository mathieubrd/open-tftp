#include <tftp.h>

int tftp_make_ack(char *buffer, size_t *length, uint16_t block) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || block < 1) {
    fprintf(stderr, "tftp_make_ack : argument invalide.\n");
    return -1;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = ACK;
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, &block, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  
  return 0;
}

int tftp_make_rrq(char *buffer, size_t *length, const char *file) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || file == NULL) {
    fprintf(stderr, "tftp_make_rrq : argument invalide.\n");
    return -1;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = RRQ;
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, file, strlen(file) + 1);
  *length += sizeof(char) * (strlen(file) + 1);
  memcpy(buffer + *length, "octet", strlen("octet") + 1);
  *length += sizeof(char) * (strlen("octet") + 1);
  
  return 0;
}

int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || block < 1 || data == NULL || n > 508) {
    fprintf(stderr, "tftp_make_data : argument invalide.\n");
    return -1;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = DATA;
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, &block, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, data, n);
  *length += n;
  
  return 0;
}

int tftp_make_error(char *buffer, size_t *length, uint16_t errorcode, const char *message) {  
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || message == NULL) {
    fprintf(stderr, "tftp_make_error : argument invalide.\n");
    return -1;
  }
  
  // Vérifie le code d'erreur
  if (errorcode != UNDEF && errorcode != FILNF && errorcode != ILLEG && errorcode != UNKNW) {
    fprintf(stderr, "tftp_make_error : code d'erreur invalide.\n");
    return -1;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = ERROR;
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, &errorcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, message, strlen(message) + 1);
  *length += strlen(message) + 1;
  
  return 0;
}

int tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || msg == NULL) {
    fprintf(stderr, "tftp_send_error : argument invalide.\n");
    return -1;
  }
  
  // Construit le paquet ERROR
  size_t length = 512;
  char buffer[length];
  if (tftp_make_error(buffer, &length, code, msg) != 0) {
    return -1;
  }
  
  // Envoie le paquet ERROR
  if (writeToSocketUDP(socket, dst, buffer, length) == -1) {
    fprintf(stderr, "tftp_make_error : impossible d'envoyer le paquet. ERROR\n");
    return -1;
  }
  
  return 0;
}

int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connection, char *response, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || file == NULL || connection == NULL || response == NULL || reslen == NULL) {
    fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout : argument invalide.\n");
    return -1;
  }
  
  // Construit le paquet RRQ
  size_t length = 512;
  char buffer[length];
  if (tftp_make_rrq(buffer, &length, file) != 0) {
    return -1;
  }
  
  // Envoie le paquet RRQ
  if (writeToSocketUDP(socket, dst, buffer, length) == -1) {
    fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout : impossible d'envoyer le paquet RRQ.\n");
    return -1;
  }
  
  // Attend la réponse
  AdresseInternet con_buf;
  char res_buf[512];
  ssize_t count;
  if ((count = recvFromSocketUDP(socket, res_buf, 512, &con_buf, TIMEOUT)) == -1) {
    fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout : impossible de recevoir le paquet.\n");
    return -1;
  }
  
  // Vérifie que le paquet reçu est de type DATA
  uint16_t opcode;
  memcpy(&opcode, res_buf, sizeof(uint16_t));
  if (opcode != DATA) {
    // Si le paquet reçu est de type ERROR, l'affiche
    if (opcode == ERROR) {
      tftp_print_error(res_buf);
    } else {
      fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout : le paquet reçu n'est pas de type DATA.\n");
      return -1;
    }
  }
  
  if (AdresseInternet_copy(connection, &con_buf) != 0) {
    fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout : impossible de copier la structure AdresseInternet.\n");
    return -1;
  }
  memcpy(response, res_buf, count);
  *reslen = count;
  
  return 0;
  
}

int tftp_send_RRQ_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connection, char *response, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || file == NULL || connection == NULL || response == NULL || reslen == NULL) {
    fprintf(stderr, "tftp_send_RRQ_wait_DATA : argument invalide.\n");
    return -1;
  }
  
  // Envoie le paquet RRQ avec plusieurs essais en cas d'echec
  unsigned int tries = 0;
  do {
    if (tftp_send_RRQ_wait_DATA_with_timeout(socket, dst, file, connection, response, reslen) == 0) {
      break;
    }
    
    tries++;
    
  } while (tries < MAX_TRIES);
  
  return 0;
}

int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL || packlen > 512) {
    fprintf(stderr, "tftp_send_DATA_wait_ACK: argument illégal.\n");
    return -1;
  }
  
  // Vérifie que le paquet donné et de type DATA
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  if (opcode != DATA) {
    fprintf(stderr, "tftp_send_DATA_wait_ACK: le paquet donné n'est pas de type DATA.\n");
    return -1;
  }
  
  // Envoie le paquet DATA
  size_t tries = 0;
  while (tries <= MAX_TRIES) {
    if (writeToSocketUDP(socket, dst, packet, packlen) < 0) {
      fprintf(stderr, "tftp_send_DATA_wait_ACK: impossible d'envoyer le paquet DATA.\n");
      return -1;
    }
    
    // Attend un paquet
    size_t length = 512;
    char buffer[length];
    if (recvFromSocketUDP(socket, buffer, length, NULL, TIMEOUT) < 0) {
      fprintf(stderr, "tftp_send_DATA_wait_ACK: impossible de recevoir le paquet.\n");
      return -1;
    }
    
    // Vérifie si c'est un paquet ACK
    memcpy(&opcode, buffer, sizeof(uint16_t));
    if (opcode != ACK) {
      // Si le paquet reçu est de type ERROR, l'affiche
      if (opcode == ERROR) {
	tftp_print_error(buffer);
      } else {
	fprintf(stderr, "tftp_send_DATA_wait_ACK: le paquet reçu n'est pas de type ACK.\n");
	tftp_send_error(socket, dst, ILLEG, "Un paquet ACK été attendu.");
      }
    } else {
      // Vérifie si les numéros de bloc correspondent
      uint16_t blockDATA;
      memcpy(&blockDATA, packet + sizeof(uint16_t), sizeof(uint16_t));
      uint16_t blockACK;
      memcpy(&blockACK, buffer + sizeof(uint16_t), sizeof(uint16_t));
      
      if (blockDATA == blockACK) {
	return 0;
      } else {
	fprintf(stderr, "tftp_send_DATA_wait_ACK: les numéros de bloc ne correspondent pas.\n");
      }
    }
    
    tries++;
  }
  
  fprintf(stderr, "tftp_send_DATA_wait_ACK: aucune paquet ACK valide reçu.\n");
  return -1;
}

int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen, char *res, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL || packlen > 512 || res == NULL || reslen == NULL) {
    fprintf(stderr, "tftp_send_ACK_wait_DATA : argument invalide.\n");
    return -1;
  }
  
  // Vérifie si le paquet donné est de type ACK
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  if (opcode != ACK) {
    fprintf(stderr, "tftp_send_ACK_wait_DATA : le paquet donné n'est pas de type ACK\n");
    return -1;
  }
  
  // Envoie le paquet ACK
  if (writeToSocketUDP(socket, dst, packet, packlen) == -1) {
    fprintf(stderr, "tftp_send_ACK_wait_DATA : impossible d'envoyer le paquet ACK\n");
    return -1;
  }
  
  // Attend le paquet DATA
  ssize_t length = 512;
  char buffer[length];
  AdresseInternet from;
  if ((length = recvFromSocketUDP(socket, buffer, length, &from, TIMEOUT)) == -1) {
    fprintf(stderr, "tftp_send_ACK_wait_DATA : impossible de recevoir le paquet.\n");
    return -1;
  }
  
  // Vérifie si c'est un packet DATA
  memcpy(&opcode, buffer, sizeof(uint16_t));
  if (opcode != DATA) {
    // Si le paquet reçu est de type ERROR, l'affiche
    if (opcode == ERROR) {
      tftp_print_error(buffer);
    } else {
      fprintf(stderr, "tftp_send_ACK_wait_DATA : le paquet reçu n'est pas de type DATA.\n");
      return -1;
    }
  }
  
  memcpy(res, buffer, length);
  *reslen = (size_t) length;

  return 0;
}

int tftp_send_last_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL || packlen > 512) {
    return -1;
  }
  
  // Vérifie que le paquet donné est de type ACK
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  if (opcode != ACK) {
    fprintf(stderr, "tftp_send_last_ACK: le paquet donné n'est pas de type ACK.\n");
    return -1;
  }
  
  // Envoie le paquet ACK
  if (writeToSocketUDP(socket, dst, packet, packlen) == -1) {
    fprintf(stderr, "tftp_send_last_ACK: impossible de recevoir le paquet.\n");
    return -1;
  }
  
  return 0;
}

void tftp_print_error(char *buffer) {
  // Vérifie les arguments
  if (buffer == NULL) {
    return;
  }
  
  // Vérifie le type du paquet
  uint16_t opcode;
  memcpy(&opcode, &buffer[0], sizeof(uint16_t));
  if (opcode != ERROR) {
    return;
  }
  
  // Affiche l'erreur sur la sortie standard
  uint16_t errcode;
  memcpy(&errcode, buffer + sizeof(uint16_t), sizeof(uint16_t));
  char *errmsg = buffer + sizeof(uint16_t) * 2;
  printf("Erreur reçue :\n");
  printf("Code d'erreur : %d\n", errcode);
  printf("Message : %s\n", errmsg);
}
