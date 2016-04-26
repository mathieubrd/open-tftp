#include <tftp.h>

static char *errors[] = {
  NULL,
  "Argument invalide",
  "Code d'erreur invalide",
  "Impossible d'envoyer le paquet",
  "Impossible de recevoir le paquet",
  "Le délai d'attendre est dépassé",
  "Le paquet donné est invalide",
  "Erreur inconnue",
  "Aucun paquet reçu",
};

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

int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n) {
  // Vérifie les arguments
  if (buffer == NULL || length == NULL || *length > 512 || block < 1 || data == NULL || n > 508) {
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
  char res_buf[512];
  size_t count;
  if ((count = recvFromSocketUDP(socket, res_buf, 512, &con_buf, TIMEOUT)) == (size_t) -1) {
    if (errno == EINTR) {
      return ERECE;
    } else {
      return ERECE;
    }
  }
  
  // Vérifie que le paquet reçu est de type DATA
  uint16_t opcode;
  memcpy(&opcode, res_buf, sizeof(uint16_t));
  opcode = ntohs(opcode);
  if (opcode != DATA) {
    return ENOPA;
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
  if (socket == NULL || dst == NULL || packet == NULL || packlen > 512) {
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
  while (tries <= MAX_TRIES) {
    if (writeToSocketUDP(socket, dst, packet, packlen) < 0) {
      return ESEND;
    }
    
    // Attend un paquet
    size_t length = 512;
    char buffer[length];
    if (recvFromSocketUDP(socket, buffer, length, NULL, TIMEOUT) < 0) {
      return ERECE;
    }
    
    // Vérifie si c'est un paquet ACK
    memcpy(&opcode, buffer, sizeof(uint16_t));
    opcode = ntohs(opcode);
    if (opcode != ACK) {
      tftp_send_error(socket, dst, ILLEG, "Un paquet ACK été attendu.");
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
    
    tries++;
  }
  
  return ENOPA;
}

int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen, char *res, size_t *reslen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL || packlen > 512 || res == NULL || reslen == NULL) {
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
  size_t length = 512;
  char buffer[length];
  AdresseInternet from;
  if ((length = recvFromSocketUDP(socket, buffer, length, &from, TIMEOUT)) == (size_t) -1) {
    if (errno == EINTR) {
      return ETIMO;
    } else {
      return ERECE;
    }
  }
  
  // Vérifie si c'est un packet DATA
  memcpy(&opcode, buffer, sizeof(uint16_t));
  if (opcode != htons(DATA)) {
    return ENOPA;
  }
  
  memcpy(res, buffer, length);
  *reslen = (size_t) length;

  return 0;
}

int tftp_send_last_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen) {
  // Vérifie les arguments
  if (socket == NULL || dst == NULL || packet == NULL || packlen > 512) {
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
