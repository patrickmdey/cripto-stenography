#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <openssl/bio.h>

#define BUFF_INC 1024
#define DES_BLOCK_SIZE 8
#define BITS 8

// TODO VER CAMBIAR
#define MAX_AES_KEY 32

typedef const EVP_CIPHER* (*encrypt_algo)(void);

static encrypt_algo encrypt_algo_map[4][4] = {
    {EVP_aes_128_cbc, EVP_aes_128_ecb, EVP_aes_128_ofb, EVP_aes_128_cfb1},
    {EVP_aes_192_cbc, EVP_aes_192_ecb, EVP_aes_192_ofb, EVP_aes_192_cfb1},
    {EVP_aes_256_cbc, EVP_aes_256_ecb, EVP_aes_256_ofb, EVP_aes_256_cfb1},
    {EVP_des_cbc, EVP_des_ecb, EVP_des_ofb, EVP_des_cfb1}
};

enum algo_t {aes128 = 0, aes192, aes256, des};
enum mode_t {cbc = 0, ecb, ofb, cfb};

// static unsigned char * padding(unsigned char *in, int *inl, size_t blocksize);
int saveEncryptedData(unsigned char *out, int len, unsigned char *where);
void mostrarKey(unsigned char key[], int len);

int encrypt(stegobmp_configuration_ptr config) {
    int in_fd = open(config->in_file, O_RDONLY);
    // READ from file descriptor until EOF
    char * file_data = malloc(sizeof(char) * BUFF_INC);
    int read_bytes;
    int total_read_chars = 0;
    int buff_len = BUFF_INC;

    while ((read_bytes = read(in_fd, file_data, BUFF_INC)) > 0) {
        buff_len += BUFF_INC;
        file_data = realloc(file_data, buff_len);
        if (file_data == NULL) {
            printf("Failed allocating memory\n");
            exit(1);
        }

        total_read_chars += read_bytes;
    }
    
    if (read_bytes == -1) {
        printf("ERROR\n");
        free(file_data);
        exit(0);
    }
    
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
    printf("%s\n",concat);
    concat_size = strlen(concat);
    free(file_data);

    enum algo_t algo;
    enum mode_t mode;

    int is_iv_null = 0;

    if(strcmp(config->encryption_algo, "des") == 0){
        algo = des;
    } 
    else if(strcmp(config->encryption_algo, "aes128") == 0){
        algo = aes128;
    } 
    else if(strcmp(config->encryption_algo, "aes192") == 0){
        algo = aes192;
    } 
    else if(strcmp(config->encryption_algo, "aes256") == 0){
        algo = aes256;
    } 
    else {
        printf("ERROR\n");
        exit(0);
    }

    if(strcmp(config->encryption_mode, "cbc") == 0){
        mode = cbc;
    } 
    else if(strcmp(config->encryption_mode, "ecb") == 0){
        is_iv_null = 1;
        mode = ecb;
    } 
    else if(strcmp(config->encryption_mode, "ofb") == 0){
        mode = ofb;
    } 
    else if(strcmp(config->encryption_mode, "cfb") == 0){
        mode = cfb;
    } 
    else {
        printf("ERROR\n");
        exit(0);
    }

    const EVP_CIPHER * cipher = encrypt_algo_map[algo][mode]();
    EVP_CIPHER_CTX *ctx;

    /*inicializar contexto*/ 
    ctx = EVP_CIPHER_CTX_new();

    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
    EVP_BytesToKey(cipher, EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 1, key, iv);

    unsigned char out_a[concat_size + 32]; //TODO chequear max block size
    int out_length, temp_length;

    EVP_CipherInit_ex(ctx, cipher, NULL, key, is_iv_null ? NULL : iv, 1);

    EVP_CipherUpdate(ctx, out_a, &out_length, (unsigned char *)concat, concat_size);
    EVP_CipherFinal(ctx, out_a + out_length, &temp_length);

    saveEncryptedData(out_a, out_length + temp_length, (unsigned char *) "base64.txt");

    EVP_CIPHER_CTX_free(ctx);

    free(concat);
    return 0;

    /*

    if(strcmp(config->encryption_algo, "des") == 0){ 
        printf("Using DES ");

        DES_cblock des_cblock;
        DES_key_schedule des_key_schedule;

        int ret;
        do {
            DES_random_key(&des_cblock);
            DES_set_odd_parity(&des_cblock);
            ret = DES_set_key_checked(&des_cblock, &des_key_schedule);
        } while(ret < 0);

        unsigned char * out = calloc(concat_size + DES_BLOCK_SIZE, sizeof(char));
        if (strcmp(config->encryption_mode, "cbc") == 0) {
            printf("with cbc\n");
            unsigned char * inPad = padding((unsigned char *)concat, &concat_size, DES_BLOCK_SIZE);
            DES_ncbc_encrypt((unsigned char *) inPad, out, concat_size, &des_key_schedule, &des_cblock, DES_ENCRYPT);
            free(inPad);
        }
        else if (strcmp(config->encryption_mode, "ecb") == 0) {
            printf("with ecb\n");
            unsigned char * inPad = padding((unsigned char *)concat, &concat_size, DES_BLOCK_SIZE);

            DES_cblock inB;
            DES_cblock outB;
            int numB;
            int i;
            numB = concat_size / DES_BLOCK_SIZE;
            for (i = 0; i < numB; i++) {
                memcpy(inB, inPad + i * DES_BLOCK_SIZE, DES_BLOCK_SIZE);
                DES_ecb_encrypt(&inB, &outB, &des_key_schedule, DES_ENCRYPT);
                memcpy(out + i * DES_BLOCK_SIZE, outB, DES_BLOCK_SIZE);
            }
            free(inPad);
        }
        else if (strcmp(config->encryption_mode, "ofb") == 0) {
            printf("with ofb\n");
            DES_ofb_encrypt((unsigned char *) concat, out, BITS, concat_size, &des_key_schedule, &des_cblock);
        }
        else if (strcmp(config->encryption_mode, "cfb") == 0) {
            printf("with cfb\n");
            DES_cfb_encrypt((unsigned char *) concat, out, BITS, concat_size, &des_key_schedule, &des_cblock, DES_ENCRYPT);
        }

        printf("%s\n", out);
        free(out);
    }
    else {
        unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

        AES_KEY aes_key;
        int i;
        
        int aux = concat_size;
        unsigned char * inPad = padding((unsigned char *)concat, &concat_size, AES_BLOCK_SIZE);
        unsigned char * out = calloc(concat_size + AES_BLOCK_SIZE, sizeof(char));

        if (strcmp(config->encryption_algo, "aes128") == 0) {
            printf("Using aes128 ");
            i = EVP_BytesToKey(EVP_aes_128_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 1, key, iv);
            mostrarKey(key, EVP_CIPHER_key_length(EVP_aes_128_cbc()));
            mostrarKey(iv, EVP_CIPHER_key_length(EVP_aes_128_cbc()));
            if (i != 16) {
                printf("Key size is %d bits - should be 128 bits\n", i);
                return -1;
            }

            unsigned char out_a[concat_size + AES_BLOCK_SIZE];
            int out_length, temp_length;

            EVP_CipherInit_ex(ctx, EVP_aes_128_cbc(),NULL, key, iv, 1);
            EVP_CipherUpdate(ctx, out_a, &out_length, (unsigned char *)concat, aux);
            EVP_CipherFinal(ctx, out_a + out_length, &temp_length);
            saveEncryptedData(out_a, out_length + temp_length, (unsigned char *) "base64.txt");
            printf("hola\n");
            EVP_CIPHER_CTX_free(ctx);
            printf("antes\n");
            free(out);
            printf("despues\n");
            return 1;
        }
        else if (strcmp(config->encryption_algo, "aes192") == 0) {
            printf("Using aes192 ");
            
            i = EVP_BytesToKey(EVP_aes_192_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 12, key, iv);

            if (i != 24) {
                printf("Key size is %d bits - should be 192 bits\n", i);
                return -1;
            }

            AES_set_encrypt_key(key, 24 * BITS, &aes_key);

        }
        else if (strcmp(config->encryption_algo, "aes256") == 0) {
            printf("Using aes256 ");

            i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 14, key, iv);
            if (i != 32) {
                printf("Key size is %d bits - should be 256 bits\n", i);
                return -1;
            }

            AES_set_encrypt_key(key, 32 * BITS, &aes_key);

        }
        else {
            return -1;
        }

        if (strcmp(config->encryption_mode, "cbc") == 0) {

            // printf("with cbc\n");

            AES_cbc_encrypt((unsigned char *) inPad, out, concat_size, &aes_key, (unsigned char *)iv, AES_ENCRYPT);

            saveEncryptedData(out, concat_size, (unsigned char *) "base64.txt");
        }
        else if (strcmp(config->encryption_mode, "ecb") == 0) {

            printf("with ecb\n");
            AES_ecb_encrypt((unsigned char *) inPad, out, &aes_key, AES_ENCRYPT);
        }
        else if (strcmp(config->encryption_mode, "ofb") == 0) {
            printf("with ofb\n");
            int num = 0;
            AES_ofb128_encrypt((unsigned char *) inPad, out, concat_size, &aes_key, (unsigned char *) iv, &num);
        }
        else if (strcmp(config->encryption_mode, "cfb") == 0) {

            printf("with cfb\n");
            int num = 0;
            AES_cfb8_encrypt((unsigned char *) inPad, out, concat_size, &aes_key, (unsigned char *) iv,  &num, AES_ENCRYPT);
        }

        printf("%s", out);
        free(inPad);
        free(out);
    }
    free(concat);
    return 0;
    */
}

/*
static unsigned char * padding(unsigned char *in, int *inl, size_t blocksize) {
    int pad;
    int i;
    unsigned char *inPad;
    pad = blocksize - (*inl) % blocksize;
    inPad = malloc(*inl + pad);
    memcpy(inPad, in, *inl);
    for (i = (*inl); i < (*inl + pad); i++)
        inPad[i] = pad;
    *inl += pad;
    return (inPad);
}
*/

void mostrarKey(unsigned char key[], int len) {     
    int i;     
    for (i = 0; i < len; i++){
        printf("%0x", key[i]);     
    } 
    printf("\n");
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
