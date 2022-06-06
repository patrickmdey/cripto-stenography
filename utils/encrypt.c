#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFF_INC 1024
#define DES_BLOCK_SIZE 8
static unsigned char * padding(unsigned char *in, int *inl, size_t blocksize);


int encrypt(stegobmp_configuration_ptr config) {
    int in_fd = open(config->in_file, O_RDONLY);
    // READ from file descriptor until EOF
    char * file_data = malloc(sizeof(char) * BUFF_INC);
    int read_bytes;
    int total_read_chars = 0;
    int buff_len = BUFF_INC;


    while ((read_bytes = read(in_fd, file_data, BUFF_INC)) > 0) {
        buff_len += BUFF_INC;
        file_data= realloc(file_data, buff_len);
        if (file_data== NULL) {
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
    extension = strtok(config->in_file, ".");
    extension = strtok(NULL, ".");

    int concat_size = 10 + total_read_chars + strlen(extension) + 1 + 1; // TODO cambiar el 1
    char *concat = calloc(concat_size, sizeof(char));

    strcat(concat, file_size);
    strcat(concat, file_data);
    strcat(concat, ".");
    strcat(concat, extension);
    printf("%s\n",concat);
    concat_size = strlen(concat);
    free(file_data);

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
            printf("with ebc\n");
            // ???
            // DES_ecb_encrypt((const unsigned char *input, unsigned char *output, long length, DES_key_schedule *schedule, DES_cblock *ivec, int enc);
        }
        else if (strcmp(config->encryption_mode, "ofb") == 0) {
            printf("with ofb\n");
            DES_ofb_encrypt((unsigned char *) concat, out, 8, concat_size, &des_key_schedule, &des_cblock);
        }
        else if (strcmp(config->encryption_mode, "cfb") == 0) {
            printf("with cfb\n");
            DES_cfb_encrypt((unsigned char *) concat, out, 8, concat_size, &des_key_schedule, &des_cblock, DES_ENCRYPT);
        }

        printf("%s\n", out);
        free(out);
    }
    else {
        unsigned char key[16], iv[16];//, iv[16];
        // char *iv = "0123456789012345"; //128 bits

        AES_KEY aes_key;
        EVP_BytesToKey(EVP_aes_128_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
        // AES_set_encrypt_key((const unsigned char*)config->password, strlen((const char *)config->password) * 8, &aes_key);
        int i = 16;

        unsigned char * inPad = padding((unsigned char *)concat, &concat_size, AES_BLOCK_SIZE);
        unsigned char * out = calloc(concat_size + AES_BLOCK_SIZE, sizeof(char));
        if (strcmp(config->encryption_algo, "aes128") == 0) {

            printf("Using aes128 ");
            // i = EVP_BytesToKey(EVP_aes_128_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
            AES_set_encrypt_key(key, 16*8, &aes_key);

            if (i != 16) {
                printf("Key size is %d bits - should be 128 bits\n", i);
                return -1;
            }
        }
        else if (strcmp(config->encryption_algo, "aes192") == 0) {

            printf("Using aes192 ");
            
            i = EVP_BytesToKey(EVP_aes_192_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
            if (i != 24) {
                printf("Key size is %d bits - should be 192 bits\n", i);
                return -1;
            }
        }
        else if (strcmp(config->encryption_algo, "aes256") == 0) {

            printf("Using aes256 ");

            i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
            if (i != 32) {
                printf("Key size is %d bits - should be 256 bits\n", i);
                return -1;
            }
        }
        else {

        }

        if (strcmp(config->encryption_mode, "cbc") == 0) {

            printf("with cbc\n");

            AES_cbc_encrypt((unsigned char *) inPad, out, concat_size, &aes_key, (unsigned char *)iv, AES_ENCRYPT);
            free(inPad);
        }
        else if (strcmp(config->encryption_algo, "ecb") == 0) {

            printf("with ecb\n");
            AES_ecb_encrypt((unsigned char *) concat, out, &aes_key, AES_ENCRYPT);
        }
        else if (strcmp(config->encryption_algo, "ofb") == 0) {

            printf("with ofb\n");
            /* set where on the 128 bit encrypted block to begin encryption*/
            int num = 0;
            AES_ofb128_encrypt((unsigned char *) concat, out, concat_size, &aes_key, (unsigned char *) iv, &num);
        }
        else if (strcmp(config->encryption_algo, "cfb") == 0) {

            printf("with cfb\n");
            /* set where on the 128 bit encrypted block to begin encryption*/
            int num = 0;
            AES_cfb8_encrypt((unsigned char *) concat, out, concat_size, &aes_key, (unsigned char *) iv,  &num, AES_ENCRYPT);
        }

        printf("%s\n", out);
        free(out);
    }
    free(concat);
    return 0;
}



static unsigned char * padding(unsigned char *in, int *inl, size_t blocksize) {
    int pad;
    int i;
    unsigned char *inPad;
    pad = blocksize - (*inl) % blocksize;
    inPad = malloc(*inl + pad);
    memcpy(inPad, in, *inl);
    for (i = (*inl); i < (*inl + pad); i++)
        inPad[i] = pad;
    *inl +=pad;
    return (inPad);
}
