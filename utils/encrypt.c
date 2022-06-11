#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <endian.h>


#define BUFF_INC 1024
#define MAX_BLOCK_SIZE 32
#define BITS 8

// TODO VER CAMBIAR
#define MAX_AES_KEY 32

typedef const EVP_CIPHER* (*encrypt_algo)(void);

static encrypt_algo encrypt_algo_map[4][4] = {
    {EVP_aes_128_cbc, EVP_aes_128_ecb, EVP_aes_128_ofb, EVP_aes_128_cfb8},
    {EVP_aes_192_cbc, EVP_aes_192_ecb, EVP_aes_192_ofb, EVP_aes_192_cfb8},
    {EVP_aes_256_cbc, EVP_aes_256_ecb, EVP_aes_256_ofb, EVP_aes_256_cfb8},
    {EVP_des_cbc, EVP_des_ecb, EVP_des_ofb, EVP_des_cfb8}
};

enum algo_t {aes128 = 0, aes192, aes256, des};
enum mode_t {cbc = 0, ecb, ofb, cfb};

static int set_encrypt_algo(const char *encryption_algo);
static int set_encrypt_mode(const char *encryption_mode, int *is_iv_null);
int saveEncryptedData(unsigned char *out, int len, unsigned char *where);

char * encrypt(stegobmp_configuration_ptr config, char * data, uint32_t data_length, uint32_t * cipher_length, uint8_t is_encryption) {
    char * extension;
    char * token;

    token = strtok(config->in_file, ".");

    while (token != NULL) {
        extension = token;
        token = strtok(NULL, ".");
    }
    
    int concat_size = sizeof(uint32_t) + data_length + 1 + strlen(extension) + 1; // el + 1 es del .
    char concat[concat_size];

    memset(concat, 0, concat_size);


    if(is_encryption) {
        uint32_t big_endian_size = htobe32(data_length);
        memcpy(concat, &big_endian_size, sizeof(uint32_t));
        strcat(concat + sizeof(uint32_t), data);
        strcat(concat + sizeof(uint32_t) + data_length, ".");
        strcat(concat + sizeof(uint32_t) + data_length + 1, extension);
    } else  {
        memcpy(concat, data, data_length);
        concat_size = data_length;
    }

    free(data);

    enum algo_t algo;
    enum mode_t mode;

    int is_iv_null = 0;

    algo = set_encrypt_algo(config->encryption_algo);
    if (algo < 0) {
        printf("ALGO %d\n", algo);
        printf("Invalid encryption algorithm\n"); 
        return 0;
    }

    mode = set_encrypt_mode(config->encryption_mode, &is_iv_null);
    if (mode < 0) {
        printf("Invalid encryption mode\n"); 
        return 0;
    }

    const EVP_CIPHER * cipher = encrypt_algo_map[algo][mode]();
    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();

    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
    EVP_BytesToKey(cipher, EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 1, key, iv);

    unsigned char * cipher_result = calloc(concat_size + MAX_BLOCK_SIZE, sizeof(char));
    int cipher_result_length, last_block_length;

    EVP_CipherInit_ex(ctx, cipher, NULL, key, is_iv_null ? NULL : iv, is_encryption);

    EVP_CipherUpdate(ctx, cipher_result, &cipher_result_length, (unsigned char *)concat, concat_size);
    EVP_CipherFinal(ctx, cipher_result + cipher_result_length, &last_block_length);

    saveEncryptedData(cipher_result, cipher_result_length + last_block_length, (unsigned char *) "base64.txt");

    EVP_CIPHER_CTX_free(ctx);

    *cipher_length = cipher_result_length + last_block_length;
    return (char *) cipher_result;
}

static int set_encrypt_algo(const char *encryption_algo) {
    if(strcmp(encryption_algo, "des") == 0) {
        return des;
    } 
    else if(strcmp(encryption_algo, "aes128") == 0) {
        return aes128;
    } 
    else if(strcmp(encryption_algo, "aes192") == 0) {
        return aes192;
    } 
    else if(strcmp(encryption_algo, "aes256") == 0) {
        return aes256;
    } 
    else {
        return -1;
    }
}

static int set_encrypt_mode(const char *encryption_mode, int *is_iv_null){
    if(strcmp(encryption_mode, "cbc") == 0) {
        return cbc;
    } 
    else if(strcmp(encryption_mode, "ecb") == 0) {
        *is_iv_null = 1;
        return ecb;
    } 
    else if(strcmp(encryption_mode, "ofb") == 0) {
        return ofb;
    } 
    else if(strcmp(encryption_mode, "cfb") == 0) {
        return cfb;
    } 
    else {
        return -1;
    }
}

int saveEncryptedData(unsigned char *out, int len, unsigned char *where) {
    BIO *b64;
    BIO *bio;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_file());
    if(bio == NULL)
        return -1; 

    if(!BIO_write_filename(bio, where))
        return -1;
        
    bio = BIO_push(b64, bio);
    BIO_write(bio, out, len);
    BIO_flush(bio);
    BIO_free_all(bio);
    return 0;
}
