#ifndef STEG_H
#define STEG_H

#include <data_structures.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <data_structures.h>
#include <fcntl.h>
#include <utils.h>

#define HEADER_SIZE 54

int steg(stegobmp_configuration_ptr config, char *embed_data , uint32_t embed_data_length);
int steg_extract(stegobmp_configuration_ptr config);

#endif
