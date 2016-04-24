#include <stdlib.h>
#include <stdio.h>

#include <tftp.h>

int main(void) {
    size_t length = 512;
    char buffer[512];
    size_t errcode;
    
    if ((errcode = tftp_make_oack(buffer, &length, (size_t) 1024, (size_t) 512)) != (size_t) 0) {
        fprintf(stderr, "tftp_make_oack : %s\n", tftp_strerror(errcode));
        exit(EXIT_FAILURE);
    }
    
    uint16_t opcode;
    memcpy(&opcode, buffer, sizeof(uint16_t));
    opcode = ntohs(opcode);
    
    length = 0;
    printf("opcode: %d\n", opcode);
    length += sizeof(uint16_t);
    printf("opt 1: %s\n", buffer + length);
    length += sizeof(char) * strlen(buffer + length) + 1;
    printf("value 1: %s\n", buffer + length);
    length += sizeof(char) * strlen(buffer + length) + 1;
    printf("opt 2: %s\n", buffer + length);
    length += sizeof(char) * strlen(buffer + length) + 1;
    printf("value 2: %s\n", buffer + length);
    
    return EXIT_SUCCESS;
}
