#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <endian.h>
#include <logger.h>


#define BUFF_INC                1024
#define MAX_BLOCK_SIZE          32
#define ALGO_MAP_SIZE           4
#define MAX_EXTENSION_LENGTH    255


typedef const EVP_CIPHER * (*encrypt_algo)(void);

static encrypt_algo encrypt_algo_map[ALGO_MAP_SIZE][ALGO_MAP_SIZE] = {
    {EVP_aes_128_cbc, EVP_aes_128_ecb, EVP_aes_128_ofb, EVP_aes_128_cfb8},
    {EVP_aes_192_cbc, EVP_aes_192_ecb, EVP_aes_192_ofb, EVP_aes_192_cfb8},
    {EVP_aes_256_cbc, EVP_aes_256_ecb, EVP_aes_256_ofb, EVP_aes_256_cfb8},
    {EVP_des_cbc, EVP_des_ecb, EVP_des_ofb, EVP_des_cfb8}
};

enum algo_t { aes128 = 0, aes192, aes256, des };
enum mode_t { cbc = 0, ecb, ofb, cfb };

static int set_encrypt_algo(const char * encryption_algo, int * key_size);
static int set_encrypt_mode(const char * encryption_mode);
int saveEncryptedData(unsigned char * out, int len, unsigned char * where);

char * encrypt(stegobmp_configuration_ptr config, char * data, uint32_t data_length, uint32_t * cipher_length, uint8_t is_encryption) {
    int concat_size = sizeof(uint32_t) + data_length + 1 + MAX_EXTENSION_LENGTH + 1;
    char concat[concat_size];
    memset(concat, 0, concat_size);

    if (is_encryption) {
        char * extension = get_extension(config->in_file);

        concat_size = sizeof(uint32_t) + data_length + 1 + strlen(extension) + 1; // el + 1 es del . el otro es del 0 de la extension

        uint32_t big_endian_size = htobe32(data_length);
        memcpy(concat, &big_endian_size, sizeof(uint32_t));

        memcpy(concat + sizeof(uint32_t), data, data_length);
        memcpy(concat + sizeof(uint32_t) + data_length, extension, strlen(extension));

        free(extension);
    }
    else {
        concat_size = data_length;
        memcpy(concat, data, concat_size);
    }

    enum algo_t algo;
    enum mode_t mode;

    int key_size;
    algo = set_encrypt_algo(config->encryption_algo, &key_size);
    if (algo < 0)
        log(FATAL, "Algorithm %s is not valid. Check help", config->encryption_algo);

    mode = set_encrypt_mode(config->encryption_mode);
    if (mode < 0)
        log(FATAL, "Mode %s is not valid. Check help", config->encryption_mode);

    const EVP_CIPHER * cipher = encrypt_algo_map[algo][mode]();
    EVP_CIPHER_CTX * ctx;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
        log(FATAL, "Error creating cipher context%s", "");

    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

    int derived_key_size;
    derived_key_size = EVP_BytesToKey(cipher, EVP_sha256(), NULL, (unsigned char *)config->password, strlen(config->password), 1, key, iv);

    if (derived_key_size != key_size)
        log(FATAL, "Error generating key%s", "");

    unsigned char * cipher_result = calloc(concat_size + MAX_BLOCK_SIZE, sizeof(char));
    if (cipher_result == NULL)
        log(FATAL, "Error allocating memory%s", "");

    int cipher_result_length = 0, last_block_length = 0;

    if (!EVP_CipherInit_ex(ctx, cipher, NULL, key, iv, is_encryption))
        log(FATAL, "Error initializing cipher%s", "");

    if (!EVP_CipherUpdate(ctx, cipher_result, &cipher_result_length, (unsigned char *)concat, concat_size))
        log(FATAL, "Error updating cipher%s", "");

    if (!EVP_CipherFinal(ctx, cipher_result + cipher_result_length, &last_block_length))
        log(FATAL, "Error finalizing cipher%s", "");

    if (!is_encryption) {
        uint32_t size = 0;
        memcpy(&size, cipher_result, sizeof(uint32_t));

        size = be32toh(size);
        *cipher_length = size;
    }
    else {
        *cipher_length = cipher_result_length + last_block_length;
    }

    EVP_CIPHER_CTX_free(ctx);

    return (char *)cipher_result;
}

static int set_encrypt_algo(const char * encryption_algo, int * key_size) {
    if (strcmp(encryption_algo, "des") == 0) {
        *key_size = 8;
        return des;
    }
    else if (strcmp(encryption_algo, "aes128") == 0) {
        *key_size = 16;
        return aes128;
    }
    else if (strcmp(encryption_algo, "aes192") == 0) {
        *key_size = 24;
        return aes192;
    }
    else if (strcmp(encryption_algo, "aes256") == 0) {
        *key_size = 32;
        return aes256;
    }

    return ERROR_VALUE;
}

static int set_encrypt_mode(const char * encryption_mode) {
    if (strcmp(encryption_mode, "cbc") == 0) {
        return cbc;
    }
    else if (strcmp(encryption_mode, "ecb") == 0) {
        return ecb;
    }
    else if (strcmp(encryption_mode, "ofb") == 0) {
        return ofb;
    }
    else if (strcmp(encryption_mode, "cfb") == 0) {
        return cfb;
    }

    return ERROR_VALUE;
}
