#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>
#define VERSION_NUMBER "0.0.0"
typedef struct stegobmp_configuration_t
{
    char *in_file;
    char *out_file;
    char *carrier_file;
    char *steg_algo;
    char *encryption_algo;
    char *encryption_mode;
    char *password;
    uint8_t is_embed;
} stegobmp_configuration;

typedef int fd;
typedef stegobmp_configuration *stegobmp_configuration_ptr;

typedef struct BMPImage * BMPImage_ptr;

extern stegobmp_configuration_ptr stegobmp_config;


#endif
