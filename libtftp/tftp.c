#include "tftp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

int tftp_make_rrq(struct tftp_rrq *buf, const char *filename, const char *mode,
    const char **opt, size_t nopt) {
    if (buf == NULL || filename == NULL || mode == NULL)
        return -1;

    buf->opcode = TFTP_RRQ;
    buf->filename = malloc(sizeof(char) * strlen(filename) + 1);
    strncpy(buf->filename, filename, strlen(filename) + 1);
    buf->mode = malloc(sizeof(char) * strlen(mode) + 1);
    strncpy(buf->mode, mode, strlen(mode) + 1);
    buf->opt = NULL;
    buf->nopt = 0;

    if (nopt > 0 && opt != NULL) {
        for (size_t i = 0; i < nopt * 2; i++) {
            if (opt[i] != NULL) {
                size_t len = sizeof(char) * (strlen(opt[i]) + 1);
                buf->opt = realloc(buf->opt, sizeof(char *) * (i + 1));
                buf->opt[i] = malloc(len);
                strncpy(buf->opt[i], opt[i], len);
                if (i % 2 == 0)
                    buf->nopt++;
            }
        }
   }

    return 0;
}

int tftp_make_data(struct tftp_data *buf, uint16_t block, uint8_t *data,
    size_t len) {
    if (buf == NULL || data == NULL)
        return -1;

    buf->opcode = TFTP_DATA;
    buf->block = block;
    buf->data = malloc(sizeof(uint8_t) * len);
    memcpy(buf->data, data, len);

    return 0;
}

int tftp_make_ack(struct tftp_ack *buf, uint16_t block) {
    if (buf == NULL)
        return -1;
    
    buf->opcode = TFTP_ACK;
    buf->block = block;

    return 0;
}

int tftp_make_oack(struct tftp_oack *buf, const char **opt, size_t nopt) {
    if (buf == NULL || opt == NULL || nopt <= 0)
        return -1;

    buf->opcode = TFTP_OACK;
    buf->opt = NULL;
    buf->nopt = 0;
    for (size_t i = 0; i < nopt * 2; i++) {
        buf->opt = realloc(buf->opt, sizeof(char *) * (i + 1));
        size_t len = sizeof(char) * (strlen(opt[i]) + 1);
        buf->opt[i] = malloc(len);
        strncpy(buf->opt[i], opt[i], len);
        if (i % 2 == 0)
            buf->nopt++;
    }

    return 0;
}

int tftp_make_error(struct tftp_error *buf, uint16_t errcode,
    const char *errmsg) {
    if (buf == NULL || errmsg == NULL)
        return -1;

    buf->opcode = TFTP_ERROR;
    buf->errcode = errcode;
    buf->errmsg = malloc(sizeof(char) * strlen(errmsg) + 1);
    strncpy(buf->errmsg, errmsg, strlen(errmsg) + 1);

    return 0;
}

int tftp_free_rrq(struct tftp_rrq *buf) {
    if (buf == NULL)
        return -1;

    free(buf->filename);
    printf("free filename\n");
    free(buf->mode);
    printf("free mode\n");
    for (size_t i = 0; i < buf->nopt * 2; i++) {
        printf("free %s\n", buf->opt[i]);
        free(buf->opt[i]);
        printf("freed\n");
    }
    free(buf->opt);

    return 0;
}
