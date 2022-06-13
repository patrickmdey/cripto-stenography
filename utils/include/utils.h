#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <data_structures.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#define BUFF_INC 1024

char * itoa(int value, char *buffer, int base);
char * read_from_file(int fd, uint32_t * read_chars);
int write_to_file(int fd, const char * buff, int bytes);
char * get_extension(char * string);

#endif
