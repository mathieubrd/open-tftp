#ifndef CONFIG_H
#define CONFIG_H

#include <sys/stat.h>

struct _config {
    int cf_fd;
    char *cf_filename;
    off_t cf_filesize;
};

typedef struct _config config;

/**
 * Ouvre le fichier de configuration donné.
 * Retourne un objet "config" ou NULL si erreur.
 */
config *config_open(char *filename);

/**
 * Ferme le fichier de configuration et libère l'objet "config"
 */
int config_close(config *cf);

char *config_get_value(config *cf, char *key);

#endif
