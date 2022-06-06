#ifndef DECRYPT_H
#define DECRYPT_H

#include <data_structures.h>
#include <fcntl.h>
#include <unistd.h>

int decrypt(stegobmp_configuration_ptr config, char * data, int data_size);

#endif
