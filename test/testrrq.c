#include <stdlib.h>
#include <stdio.h>

#include <tftp.h>

int main(void) {
    size_t length = 512;
    char buffer[512];
    size_t errcode;
    
    if ((errcode = tftp_make_rrq_opt(buffer, &length, "fichier", (size_t) 1024, (size_t) 5)) != 0) {
        fprintf(stderr, "tftp_make_rrq_opt : %s\n", tftp_strerror(errcode));
    }
    
    uint16_t opcode;
    memcpy(&opcode, buffer, sizeof(uint16_t));
    opcode = ntohs(opcode);
    
    length = 0;
    printf("OPcode: %d\n", opcode);
    length += sizeof(uint16_t);
    printf("Filename: %s\n", buffer + length);
    length += strlen(buffer + length) + 1;
    printf("mode: %s\n", buffer + length);
    length += strlen(buffer + length) + 1;
    printf("opt 1 name: %s\n", buffer + length);
    length += strlen(buffer + length) + 1;
    printf("opt 1 value: %s\n", buffer + length);
    length += strlen(buffer + length) + 1;
    printf("opt 2 name: %s\n", buffer + length);
    length += strlen(buffer + length) + 1;
    printf("opt 2 value: %s\n", buffer + length);

    return EXIT_SUCCESS;
}
