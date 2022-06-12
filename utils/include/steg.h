#ifndef STEG_H
#define STEG_H

#ifndef _DEFAULT_SOURCE

#define _DEFAULT_SOURCE

#endif
#ifndef __USE_BSD

#define __USE_BSD

#endif

#include <data_structures.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <data_structures.h>
#include <fcntl.h>
#include <utils.h>
#include <endian.h>

#define HEADER_SIZE 54

int steg(stegobmp_configuration_ptr config, char *embed_data , uint32_t embed_data_length, char * extension);
char * steg_extract(stegobmp_configuration_ptr config, char * extract_data, uint32_t extract_data_length, uint32_t * hidden_size);

#endif
