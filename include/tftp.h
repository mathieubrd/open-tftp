#ifndef TFTP_H
#define TFTP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <SocketUDP.h>
#include <AdresseInternet.h>
#include <errno.h>

// Identifiants opcode
#define RRQ 1
#define DATA 3
#define ACK 4
#define ERROR 5
#define OACK 6

// Erreurs spécifiques au protocol TFTP
#define UNDEF 0
#define FILNF 1
#define ILLEG 4
#define UNKNW 5

// Erreurs de routines
#define NERRORS 8
#define EARGU 1
#define ERRCO 2
#define ESEND 3
#define ERECE 4
#define ETIMO 5
#define EINVA 6
#define EUNKN 7
#define ENOPA 8

// Configuration du serveur
#define TIMEOUT 5
#define MAX_TRIES 3
#define MIN_BLKSIZE 8
#define MAX_BLKSIZE 65464
#define MIN_WINDOWSIZE 1
#define MAX_WINDOWSIZE 65535

// Routines qui fabriquent des paquets
int tftp_make_ack(char *buffer, size_t *length, uint16_t block);
int tftp_make_OACK(char *buffer, size_t *length, size_t nbytes, size_t nblocks);
int tftp_make_rrq(char *buffer, size_t *length, const char *file);
int tftp_make_rrq_opt(char *buffer, size_t *length, const char *fichier, size_t nbytes, size_t nblocs);
int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n);
int tftp_make_error(char *buffer, size_t *length, uint16_t errcode, const char *message);

// Routines de communication
int tftp_wait_DATA_with_timeout(SocketUDP *socket, AdresseInternet *from, char *res, size_t *reslen);
int tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg);
int tftp_send_OACK(SocketUDP *socket, const AdresseInternet *dst, char *packet, size_t packlen);
int tftp_send_RRQ_wait_OACK(SocketUDP *socket, const AdresseInternet *dst, AdresseInternet *from, char *packet, size_t packlen, char *res, size_t *reslen);
int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *fichier, AdresseInternet *connection, char *response, size_t *reslen);
int tftp_send_RRQ_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *fichier, AdresseInternet *connection, char *response, size_t *reslen);
int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen);
int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen, char *res, size_t *reslen);
int tftp_send_last_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *packet, size_t packlen);

// Routines qui affichent des paquets
int tftp_print(char *packet);
int tftp_print_RRQ(char *packet);
int tftp_print_ACK(char *packet);
int tftp_print_OACK(char *packet);
int tftp_print_DATA(char *packet);

// Routines utilitaires
int tftp_get_opt(char *packet, size_t *nbytes, size_t *nblocks);
char *tftp_strerror(ssize_t errcode);

#endif
