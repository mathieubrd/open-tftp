#ifndef TFTP_H
#define TFTP_H

#include <stdlib.h>
#include <inttypes.h>

/* Opcodes TFTP */
#define TFTP_RRQ 1
#define TFTP_DATA 3
#define TFTP_ACK 4
#define TFTP_ERROR 5
#define TFTP_OACK 5

/* Structures qui représentent des paquets TFTP */
struct tftp_rrq {
    uint16_t opcode;
    char *filename;
    char *mode;
    char **opt;
    size_t nopt;
};

struct tftp_data {
    uint16_t opcode;
    uint16_t block;
    uint8_t *data;
};

struct tftp_ack {
    uint16_t opcode;
    uint16_t block;
};

struct tftp_oack {
    uint16_t opcode;
    char **opt;
    size_t nopt;
};

struct tftp_error {
    uint16_t opcode;
    uint16_t errcode;
    char *errmsg;
};

/* Fabrique une structure qui représente un paquet RRQ.
 * Les options sont optionnelles.
 * Les options sont données dans un tableau de chaînes de caractères.
 * Le nom des options sont suivis de leur valeur.
 * nopt est le nombre d'options (sans compter les valeurs des options).
 * Libérez la structure après utilisation avec tftp_free_rrq.
 */
int tftp_make_rrq(struct tftp_rrq *buf, const char *filename, const char *mode,
    const char **opt, size_t nopt);

/* Fabrique une structure qui représente un paquet DATA.
 * Libérez la structure après utilisation avec tftp_free_data.
 */
int tftp_make_data(struct tftp_data *buf, uint16_t block, uint8_t *data,
    size_t len);

/* Fabrique une structure qui représente un paquet ACK.
 */
int tftp_make_ack(struct tftp_ack *buf, uint16_t block);

/* Fabrique une structure qui représente un paquet OACK.
 * Les options sont données dans un tableau de chaînes de caractères.
 * Le nom des options sont suivis de leur valeur.
 * nopt est le nombre d'options (sans compter les valeurs des options).
 * Libérez la structure après utilisation avec tftp_free_oack.
 */
int tftp_make_oack(struct tftp_oack *buf, const char **opt, size_t nopt);

/* Fabrique une structure qui représente un paquet ERROR.
 * Libérez la structure après utilisation avec tftp_free_error.
 */
int tftp_make_error(struct tftp_error *buf, uint16_t errcode,
    const char *errmsg);

/* Libère une structure RRQ. */
int tftp_free_rrq(struct tftp_rrq *buf);

/* Libère une structure DATA. */
int tftp_free_data(struct tftp_data *buf);

/* Libère une structure ERROR. */
int tftp_free_error(struct tftp_error *buf);

#endif
