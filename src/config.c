#include "config.h"

#include <stdlib.h>
#include <stdio.h> // TODO: remove
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

config *config_open(char *filename) {
    // Vérifie l'argument
    if (filename == NULL) {
        fprintf(stderr, "config_open: invalid argument\n");
        return NULL;
    }
    
    // Ouvre le fichier
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "config_open: unable to open file\n");
        return NULL;
    }
    
    // Récupère les informations du fichier
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "config_open: unable to obtain file informations\n");
        return NULL;
    }
    
    // Initialise un objet "config"
    config *cf = malloc(sizeof(config));
    cf->cf_fd = fd;
    cf->cf_filename = malloc(sizeof(char) * strlen(filename) + 1);
    strncpy(cf->cf_filename, filename, strlen(filename) + 1);
    cf->cf_filesize = st.st_size;
    
    return cf;
}

int config_close(config *cf) {
    // Vérifie l'argument
    if (cf == NULL) {
        fprintf(stderr, "config_close: invalid argument\n");
        return -1;
    }
    
    // Ferme le fichier
    if (close(cf->cf_fd) != 0) {
        fprintf(stderr, "config_close: unable to close file\n");
        return -1;
    }
    
    // Libère l'objet "config"
    free(cf);
    
    return 0;
}

char *config_get_value(config *cf, char *key) {
    // Vérifie les arguments
    if (cf == NULL || key == NULL) {
        fprintf(stderr, "config_get_value: invalid argument\n");
        return NULL;
    }
    
    // Déplace le curseur au début du fichier
    if (lseek(cf->cf_fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "config_get_value: unable to move file offset\n");
        return NULL;
    }
    
    // Lit le fichier
    
    
    return "lol";
}
