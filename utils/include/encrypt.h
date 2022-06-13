#ifndef ENCRYPT_H
#define ENCRYPT_H

#ifndef _DEFAULT_SOURCE

#define _DEFAULT_SOURCE

#endif
#ifndef __USE_BSD

#define __USE_BSD

#endif

#include <data_structures.h>
#include <utils.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/aes.h>
#include <openssl/des.h>

#define ENCRYPTION  1
#define DECRYPTION  0

char * encrypt(stegobmp_configuration_ptr config, char * data, uint32_t data_length, uint32_t * cipher_length, uint8_t is_encryption);

#endif
