#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <openssl/bio.h>

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

int encrypt(stegobmp_configuration_ptr config) {
    int in_fd = open(config->in_file, O_RDONLY);
    // READ from file descriptor until EOF
    // char * file_data = malloc(sizeof(char) * BUFF_INC);
    // int read_bytes;
    // int total_read_chars = 0;
    // int buff_len = BUFF_INC;

    // while ((read_bytes = read(in_fd, file_data+total_read_chars, BUFF_INC)) > 0) {
    //     buff_len += BUFF_INC;
    //     file_data = realloc(file_data, buff_len);
    //     if (file_data == NULL) {
    //         printf("Failed allocating memory\n");
    //         exit(1);
    //     }

    //     total_read_chars += read_bytes;
    // }

    int total_read_chars = 0;

    char *file_data = read_from_file(in_fd, &total_read_chars);
    // if (read_bytes == -1) {
    //     printf("ERROR\n");
    //     free(file_data);
    //     exit(0);
    // }
    
    file_data[total_read_chars] = '\0';

    char file_size[10];
    itoa(total_read_chars, file_size, 10);

    char * extension;
    char * token;

    token = strtok(config->in_file, ".");

    while (token != NULL) {
        extension = token;
        token = strtok(NULL, ".");
    }
    
    int concat_size = 10 + total_read_chars + strlen(extension) + 1 + 1; // TODO cambiar el 1
    char *concat = calloc(concat_size, sizeof(char));

    strcat(concat, file_size);
    strcat(concat, file_data);
    strcat(concat, ".");
    strcat(concat, extension);

    concat_size = strlen(concat);

    free(file_data);

    enum algo_t algo;
    enum mode_t mode;

    int is_iv_null = 0;

    algo = set_encrypt_algo(config->encryption_algo);
    if (algo < 0){
        printf("ALGO %d\n", algo);
        printf("Invalid encryption algorithm\n"); 
        free(concat); 
        return 0;
    }

    mode = set_encrypt_mode(config->encryption_mode, &is_iv_null);
    if (mode < 0){
        printf("Invalid encryption mode\n"); 
        free(concat); 
        return 0;
    }

    const EVP_CIPHER * cipher = encrypt_algo_map[algo][mode]();
    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();

    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
    EVP_BytesToKey(cipher, EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 1, key, iv);

    unsigned char out[concat_size + MAX_BLOCK_SIZE];
    int out_length, temp_length;

    EVP_CipherInit_ex(ctx, cipher, NULL, key, is_iv_null ? NULL : iv, 1);

    EVP_CipherUpdate(ctx, out, &out_length, (unsigned char *)concat, concat_size);
    EVP_CipherFinal(ctx, out + out_length, &temp_length);

    saveEncryptedData(out, out_length + temp_length, (unsigned char *) "base64.txt");

    EVP_CIPHER_CTX_free(ctx);

    free(concat);
    return 0;
}

static int set_encrypt_algo(const char *encryption_algo){
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
