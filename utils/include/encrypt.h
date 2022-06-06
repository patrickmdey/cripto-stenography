#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <data_structures.h>
#include <utils.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/aes.h>
#include <openssl/des.h>

int encrypt(stegobmp_configuration_ptr config);

#endif
