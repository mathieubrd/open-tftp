#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILE "tftp.conf"

int main(void) {
    config *cf = config_open(CONFIG_FILE);
    if (cf == NULL) {
        exit(EXIT_FAILURE);
    }
    
    printf("filename: %s\n", cf->cf_filename);
    
    if (config_close(cf) != 0) {
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}
