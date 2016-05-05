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

/* Routines qui fabriquent des structures TFTP */
int tftp_make_rrq(struct tftp_rrq *buf, const char *filename, const char *mode,
    const char **opt, size_t nopt);
int tftp_make_data(struct tftp_data *buf, uint16_t block, uint8_t *data,
    size_t len);
int tftp_make_ack(struct tftp_ack *buf, uint16_t block);
int tftp_make_oack(struct tftp_oack *buf, const char **opt, size_t nopt);
int tftp_make_error(struct tftp_error *buf, uint16_t errcode,
    const char *errmsg);

/* Routines qui libèrent des structures TFTP */
int tftp_free_rrq(struct tftp_rrq *buf);
int tftp_free_data(struct tftp_data *buf);
int tftp_free_error(struct tftp_error *buf);

#endif
