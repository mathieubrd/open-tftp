#include <tftp.h>

char *errors[] = {
  NULL,
  "Argument invalide",
  "Code d'erreur invalide",
  "Impossible d'envoyer le paquet",
  "Impossible de recevoir le paquet",
  "Le délai d'attendre est dépassé",
  "Le paquet est invalide",
  "Erreur inconnue",
  "Aucun paquet reçu",
};

int tftp_get_opt(char *packet, size_t *nbytes, size_t *nblocks) {
  // Vérifie les arguments
  if (packet == NULL) {
    return EARGU;
  }
  
  // Récupère l'opcode
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  opcode = ntohs(opcode);
  size_t offset = sizeof(uint16_t);
  
  char blksize[6];
  char windowsize[6];
  
  if (opcode == RRQ) {
    offset += strlen(packet + offset) + 1;
    offset += strlen(packet + offset) + 1;
  }
  
  if (nbytes != NULL) {
    offset += strlen(packet + offset) + 1;
    memcpy(blksize, packet + offset, strlen(packet + offset) + 1);
    *nbytes = (size_t) atoi(blksize);
    
    if (*nbytes < MIN_BLKSIZE) {
      *nbytes = MIN_BLKSIZE;
    }
    
    if (*nbytes > MAX_BLKSIZE) {
      *nbytes = MAX_BLKSIZE;
    }
  }
  
  if (nblocks != NULL) {
    offset += strlen(packet + offset) + 1;
    offset += strlen(packet + offset) + 1;
    memcpy(windowsize, packet + offset, strlen(packet + offset) + 1);
    *nblocks = (size_t) atoi(windowsize);
    
    if (*nblocks < MIN_WINDOWSIZE) {
      *nblocks = MIN_WINDOWSIZE;
    }
    
    if (*nblocks > MAX_WINDOWSIZE) {
      *nblocks = MAX_WINDOWSIZE;
    }
  }
  
  return 0;
}

int tftp_make_ack(char *buffer, size_t *length, uint16_t block) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || block < 1) {
    return EARGU;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = htons(ACK);
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  block = htons(block);
  memcpy(buffer + *length, &block, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  
  return 0;
}

int tftp_make_OACK(char *buffer, size_t *length, size_t nbytes, size_t nblocks) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL) {
    return EARGU;
  }
  
  // Modifie les options si elles sont invalides
  if (nbytes < MIN_BLKSIZE) {
    nbytes = MIN_BLKSIZE;
  } else if (nbytes > MAX_BLKSIZE) {
    nbytes = MAX_BLKSIZE;
  }
  
  if (nblocks < MIN_WINDOWSIZE) {
    nblocks = MIN_WINDOWSIZE;
  } else if (nblocks > MAX_WINDOWSIZE) {
    nblocks = MAX_WINDOWSIZE;
  }
  
  // Construit le paquet
  char snbytes[6];
  char snblocks[6];
  uint16_t opcode = htons(OACK);
  
  sprintf(snbytes, "%zu", nbytes);
  sprintf(snblocks, "%zu", nblocks);
  
  memset(buffer, 0, *length);
  *length = 0;
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  if (nbytes != (size_t) 0) {
    memcpy(buffer + *length, "blksize", strlen("blksize") + 1);
    *length += sizeof(char) * (strlen("blksize") + 1);
    memcpy(buffer + *length, snbytes, strlen(snbytes) + 1);
    *length += sizeof(char) * (strlen(snbytes) + 1);
  }
  if (nblocks != (size_t) 0) {
    memcpy(buffer + *length, "windowsize", strlen("windowsize") + 1);
    *length += sizeof(char) * (strlen("windowsize") + 1);
    memcpy(buffer + *length, snblocks, strlen(snblocks) + 1);
    *length += sizeof(char) * (strlen(snblocks) + 1);
  }
  
  return 0;
}

int tftp_make_rrq(char *buffer, size_t *length, const char *file) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || file == NULL) {
    return EARGU;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = htons(RRQ);
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, file, strlen(file) + 1);
  *length += sizeof(char) * (strlen(file) + 1);
  memcpy(buffer + *length, "octet", strlen("octet") + 1);
  *length += sizeof(char) * (strlen("octet") + 1);
  
  return 0;
}

int tftp_make_rrq_opt(char *buffer, size_t *length, const char *fichier, size_t nbytes, size_t nblocks) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || fichier == NULL) {
    return EARGU;
  }
  
  // Contruit un paquet RRQ sans option
  size_t errcode;
  if ((errcode = tftp_make_rrq(buffer, length, fichier)) != 0) {
    return errcode;
  }
  
  // Rajoute les options
  char snbytes[6];
  char snblocks[6];
  
  sprintf(snbytes, "%lu", nbytes);
  sprintf(snblocks, "%lu", nblocks);
  
  if (nbytes != (size_t) 0) {
    memcpy(buffer + *length, "blksize", strlen("blksize") + 1);
    *length += sizeof(char) * (strlen("blksize") + 1);
    memcpy(buffer + *length, snbytes, strlen(snbytes) + 1);
    *length += sizeof(char) * (strlen(snbytes) + 1);
  }
  
  if (nblocks != (size_t) 0) {
    memcpy(buffer + *length, "windowsize", strlen("windowsize") + 1);
    *length += sizeof(char) * (strlen("windowsize") + 1);
    memcpy(buffer + *length, snblocks, strlen(snblocks) + 1);
    *length += sizeof(char) * (strlen(snblocks) + 1);
  }
  
  return 0;
}

int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || data == NULL) {
    return EARGU;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = htons(DATA);
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  block = htons(block);
  memcpy(buffer + *length, &block, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, data, n);
  *length += n;
  
  return 0;
}

int tftp_make_error(char *buffer, size_t *length, uint16_t errcode, const char *message) {  
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || message == NULL) {
    return EARGU;
  }
  
  // Vérifie le code d'erreur
  if (errcode != UNDEF && errcode != FILNF && errcode != ILLEG && errcode != UNKNW) {
    return ERRCO;
  }
  
  // Construit le paquet
  memset(buffer, 0, *length);
  *length = 0;
  uint16_t opcode = htons(ERROR);
  memcpy(buffer, &opcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  errcode = htons(errcode);
  memcpy(buffer + *length, &errcode, sizeof(uint16_t));
  *length += sizeof(uint16_t);
  memcpy(buffer + *length, message, strlen(message) + 1);
  *length += strlen(message) + 1;
  
  return 0;
}

int tftp_wait_DATA_with_timeout(SocketUDP *socket, AdresseInternet *from, char *res, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || from == NULL || res == NULL || reslen == NULL) {
    return EARGU;
  }
  
  // Attend le paquet DATA
  AdresseInternet con_buf;
  char res_buf[*reslen];
  size_t count;
  if ((count = recvFromSocketUDP(socket, res_buf, *reslen, &con_buf, TIMEOUT)) == (size_t) -1) {
    if (errno == EINTR) {
      return ETIME;
    } else {
      return ERECE;
    }
  }
  
  if (AdresseInternet_copy(from, &con_buf) != 0) {
    return EUNKN;
  }
  memcpy(res, res_buf, count);
  *reslen = count;
  
  return 0;
}

int tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || msg == NULL) {
    return EARGU;
  }
  
  // Construit le paquet ERROR
  size_t length = 512;
  char buffer[length];
  size_t errcode;
  if ((errcode = tftp_make_error(buffer, &length, code, msg)) != 0) {
    return errcode;
  }
  
  // Envoie le paquet ERROR
  if (writeToSocketUDP(socket, dst, buffer, length) == -1) {
    return ESEND;
  }
  
  return 0;
}

int tftp_send_OACK(SocketUDP *socket, const AdresseInternet *dst, char *packet, size_t packlen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL) {
    return EARGU;
  }
  
  // Envoie le paquet OACK
  if (writeToSocketUDP(socket, dst, packet, packlen) == (ssize_t) -1) {
    return ESEND;
  }
  
  return 0;
}

int tftp_send_RRQ_wait_OACK(SocketUDP *socket, const AdresseInternet *dst, AdresseInternet *from, char *packet, size_t packlen, char *res, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || from == NULL || packet == NULL || res == NULL || reslen == NULL) {
    return EARGU;
  }
  
  // Envoie le paquet RRQ
  if (writeToSocketUDP(socket, dst, packet, packlen) == (ssize_t) -1) {
    return ESEND;
  }
  
  // Attend la réponse
  if (recvFromSocketUDP(socket, res, *reslen, from, -1) == (ssize_t) -1) {
    return ERECE;
  }
  
  return 0;
}

int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connection, char *response, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || file == NULL || connection == NULL || response == NULL || reslen == NULL) {
    return EARGU;
  }
  
  // Construit le paquet RRQ
  size_t length = 512;
  char buffer[length];
  size_t errcode;
  if ((errcode = tftp_make_rrq(buffer, &length, file)) != 0) {
    return errcode;
  }
  
  // Envoie le paquet RRQ
  if (writeToSocketUDP(socket, dst, buffer, length) == -1) {
    return ESEND;
  }
  
  // Attend la réponse
  AdresseInternet con_buf;
  char res_buf[*reslen];
  size_t count;
  if ((count = recvFromSocketUDP(socket, res_buf, *reslen, &con_buf, TIMEOUT)) == (size_t) -1) {
    if (errno == EINTR) {
      return ETIME;
    } else {
      return ERECE;
    }
  }
  
  if (AdresseInternet_copy(connection, &con_buf) != 0) {
    return EUNKN;
  }
  memcpy(response, res_buf, count);
  *reslen = count;
  
  return 0;
  
}

int tftp_send_RRQ_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connection, char *response, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || file == NULL || connection == NULL || response == NULL || reslen == NULL) {
    return EARGU;
  }
  
  // Envoie le paquet RRQ avec plusieurs essais en cas d'echec
  unsigned int tries = 0;
  size_t errcode;
  do {
    if ((errcode = tftp_send_RRQ_wait_DATA_with_timeout(socket, dst, file, connection, response, reslen)) == 0) {
      return 0;
    }
    
    tries++;
    
  } while (tries < MAX_TRIES);
  
  return errcode;
}

int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL) {
    return EARGU;
  }
  
  // Vérifie que le paquet donné et de type DATA
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  if (opcode != htons(DATA)) {
    return EINVAL;
  }
  
  // Envoie le paquet DATA
  size_t tries = 0;
  if (writeToSocketUDP(socket, dst, packet, packlen) < 0) {
    return ESEND;
  }
  
  while (tries <= MAX_TRIES) {
    // Attend un paquet
    size_t length = (size_t) 512;
    char buffer[length];
    if (recvFromSocketUDP(socket, buffer, length, NULL, TIMEOUT) < 0) {
      tries++;
      continue;
    }
    
    // Vérifie si c'est un paquet ACK
    memcpy(&opcode, buffer, sizeof(uint16_t));
    opcode = ntohs(opcode);
    if (opcode != ACK) {
      tftp_send_error(socket, dst, ILLEG, "Un paquet ACK été attendu.");
      tries++;
      continue;
    } else {
      // Vérifie si les numéros de bloc correspondent
      uint16_t blockDATA;
      memcpy(&blockDATA, packet + sizeof(uint16_t), sizeof(uint16_t));
      blockDATA = ntohs(blockDATA);
      uint16_t blockACK;
      memcpy(&blockACK, buffer + sizeof(uint16_t), sizeof(uint16_t));
      blockACK = ntohs(blockACK);
      if (blockDATA == blockACK) {
        return 0;
      }
    }
  }
  
  return ENOPA;
}

int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen, char *res, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL || res == NULL || reslen == NULL) {
    return EARGU;
  }
  
  // Vérifie si le paquet donné est de type ACK
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  if (opcode != htons(ACK)) {
    return EINVAL;
  }
  
  // Envoie le paquet ACK
  if (writeToSocketUDP(socket, dst, packet, packlen) == -1) {
    return ESEND;
  }
  
  // Attend le paquet DATA
  size_t tries = 0;
  
  while (tries < MAX_TRIES) {
    size_t length = *reslen;
    char buffer[length];
    AdresseInternet from;
    if ((length = recvFromSocketUDP(socket, buffer, length, &from, TIMEOUT)) == (size_t) -1) {
      tries++;
      continue;
    }
    
    memcpy(res, buffer, length);
    *reslen = (size_t) length;
    
    return 0;
  }
  
  return ETIMO;
}

int tftp_send_last_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL) {
    return EARGU;
  }
  
  // Vérifie que le paquet donné est de type ACK
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  if (opcode != htons(ACK)) {
    return EINVAL;
  }
  
  // Envoie le paquet ACK
  if (writeToSocketUDP(socket, dst, packet, packlen) == -1) {
    return ESEND;
  }
  
  return 0;
}

char *tftp_strerror(ssize_t errcode) {
  if (errcode >= 1 && errcode <= NERRORS) {
    return errors[errcode];
  } else {
    return "Aucune erreur";
  }
}

int tftp_print(char *packet) {
    // Vérifie l'argument
    if (packet == NULL) {
      return EARGU;
    }
    
    // Vérifie l'opcode
    uint16_t opcode;
    memcpy(&opcode, packet, sizeof(uint16_t));
    opcode = ntohs(opcode);
    
    // Appelle la bonne routine en fonction de l'opcode
    switch (opcode) {
      case (uint16_t) RRQ:
	tftp_print_RRQ(packet);
	break;
      
      case (uint16_t) ACK:
	tftp_print_ACK(packet);
	break;
	
      case (uint16_t) OACK:
	tftp_print_OACK(packet);
	break;
      
      case (uint16_t) DATA:
	tftp_print_DATA(packet);
	break;
	
      case (uint16_t) ERROR:
	tftp_print_ERROR(packet);
	break;
    }
    
    return EINVA;
}

int tftp_print_RRQ(char *packet) {
  // Vérifie l'argument
  if (packet == NULL) {
    return EARGU;
  }
  
  // Vérifie l'opcode
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  opcode = ntohs(opcode);
  if (opcode != (uint16_t) RRQ) {
    return EINVA;
  }
  size_t offset = sizeof(uint16_t);
  
  // Récupère le nom du fichier demandé
  char *filename = packet + offset;
  offset += strlen(packet + offset) + 1;
  
  // Récupère le mode de transmission
  char *mode = packet + offset;
  offset += strlen(packet + offset) + 1;
  
  // Affiche le paquet
  printf("RRQ - %s - %s", filename, mode);
  
  // Affiche les éventuelles options
  size_t blksize = 0;
  size_t windowsize = 0;
  tftp_get_opt(packet, &blksize, &windowsize);
  if (blksize != (size_t) 0) {
    printf(" - blksize: %lu", blksize);
  }
  if (windowsize != (size_t) 0) {
    printf(" - windowsize: %lu", windowsize);
  }
  
  printf("\n");
  
  return 0;
}

int tftp_print_ACK(char *packet) {
  // Vérifie l'argument
  if (packet == NULL) {
    return EARGU;
  }
  
  // Vérifie l'opcode
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  opcode = ntohs(opcode);
  if (opcode != (uint16_t) ACK) {
    return EINVA;
  }
  size_t offset = sizeof(uint16_t);
  
  // Récupère le numéro de bloc
  uint16_t bloc;
  memcpy(&bloc, packet + offset, sizeof(uint16_t));
  bloc = ntohs(bloc);
  
  // Affiche le paquet
  printf("ACK - %d\n", bloc);
  
  return 0;
}

int tftp_print_OACK(char *packet) {
  // Vérifie l'argument
  if (packet == NULL) {
    return EARGU;
  }
  
  // Vérifie l'opcode
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  opcode = ntohs(opcode);
  if (opcode != (uint16_t) OACK) {
    return EINVA;
  }
  
  // Récupère les options
  size_t blksize = 0;
  tftp_get_opt(packet, &blksize, NULL);
  
  // Affiche le paquet
  printf("OACK - blksize: %lu\n", blksize);
  
  return 0;
}

int tftp_print_DATA(char *packet) {
  // Vérifie l'argument
  if (packet == NULL) {
    return EARGU;
  }
  
  // Vérifie l'opcode
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  opcode = ntohs(opcode);
  if (opcode != (uint16_t) DATA) {
    return EINVA;
  }
  size_t offset = sizeof(uint16_t);
  
  // Récupère le numéro de bloc
  uint16_t bloc;
  memcpy(&bloc, packet + offset, sizeof(uint16_t));
  bloc = ntohs(bloc);
  
  // Affiche le paquet
  printf("DATA - %d - raw data\n", bloc);
  
  return 0;
}

int tftp_print_ERROR(char *packet) {
  // Vérifie l'argument
  if (packet == NULL) {
    return EARGU;
  }
  
  // Vérifie l'opcode
  uint16_t opcode;
  memcpy(&opcode, packet, sizeof(uint16_t));
  opcode = ntohs(opcode);
  if (opcode != (uint16_t) ERROR) {
    return EINVA;
  }
  size_t offset = sizeof(uint16_t);
  
  // Récupère le code d'erreur
  uint16_t errcode;
  memcpy(&errcode, packet + offset, sizeof(uint16_t));
  errcode = ntohs(errcode);
  offset += sizeof(uint16_t);
  
  // Récupère le message d'erreur
  char *err = packet + offset;
  
  // Affiche le paquet
  printf("ERROR - %d - %s\n", errcode, err);
  
  return 0;
}
