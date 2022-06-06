#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFF_INC 1024
#define DES_BLOCK_SIZE 8
#define BITS 8
#define MAX_AES_KEY 32

// portador -> archivo enc -> desenc

int decrypt(stegobmp_configuration_ptr config, char * data, int data_len) {
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

        unsigned char * out = calloc(data_len + DES_BLOCK_SIZE, sizeof(char));
        if (strcmp(config->encryption_mode, "cbc") == 0) {
            printf("with cbc\n");
            DES_ncbc_encrypt((unsigned char *) data, out, data_len, &des_key_schedule, &des_cblock, DES_DECRYPT);
        }
        else if (strcmp(config->encryption_mode, "ecb") == 0) {
            printf("with ecb\n");

            DES_cblock inB;
            DES_cblock outB;
            int numB;
            int i;
            numB = data_len / DES_BLOCK_SIZE;
            for (i = 0; i < numB; i++) {
                memcpy(inB, data + i * DES_BLOCK_SIZE, DES_BLOCK_SIZE);
                DES_ecb_encrypt(&inB, &outB, &des_key_schedule, DES_DECRYPT);
                memcpy(out + i * DES_BLOCK_SIZE, outB, DES_BLOCK_SIZE);
            }
        }
        else if (strcmp(config->encryption_mode, "ofb") == 0) {
            printf("with ofb\n");
            DES_ofb_encrypt((unsigned char *) data, out, BITS, data_len, &des_key_schedule, &des_cblock);
        }
        else if (strcmp(config->encryption_mode, "cfb") == 0) {
            printf("with cfb\n");
            DES_cfb_encrypt((unsigned char *) data, out, BITS, data_len, &des_key_schedule, &des_cblock, DES_DECRYPT);
        }

        printf("%s\n", out);
        free(out);
    }
    else {
        unsigned char key[MAX_AES_KEY], iv[MAX_AES_KEY];

        AES_KEY aes_key;
        int i;

        unsigned char * out = calloc(data_len + AES_BLOCK_SIZE, sizeof(char));

        if (strcmp(config->encryption_algo, "aes128") == 0) {
            printf("Using aes128 ");
            i = EVP_BytesToKey(EVP_aes_128_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
            AES_set_decrypt_key(key, 16 * BITS, &aes_key);

            if (i != 16) {
                printf("Key size is %d bits - should be 128 bits\n", i);
                return -1;
            }
        }
        else if (strcmp(config->encryption_algo, "aes192") == 0) {
            printf("Using aes192 ");
            
            i = EVP_BytesToKey(EVP_aes_192_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
            AES_set_decrypt_key(key, 24 * BITS, &aes_key);

            if (i != 24) {
                printf("Key size is %d bits - should be 192 bits\n", i);
                return -1;
            }
        }
        else if (strcmp(config->encryption_algo, "aes256") == 0) {
            printf("Using aes256 ");

            i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, (unsigned char *)iv);
            AES_set_decrypt_key(key, 32 * BITS, &aes_key);
            if (i != 32) {
                printf("Key size is %d bits - should be 256 bits\n", i);
                return -1;
            }
        }
        else {
            return -1;
        }

        if (strcmp(config->encryption_mode, "cbc") == 0) {

            printf("with cbc\n");

            AES_cbc_encrypt((unsigned char *) data, out, data_len, &aes_key, (unsigned char *)iv, AES_DECRYPT);
        }
        else if (strcmp(config->encryption_mode, "ecb") == 0) {

            printf("with ecb\n");
            AES_ecb_encrypt((unsigned char *) data, out, &aes_key, AES_DECRYPT);
        }
        else if (strcmp(config->encryption_mode, "ofb") == 0) {
            printf("with ofb\n");
            /* set where on the 128 bit encrypted block to begin encryption*/
            int num = 0;
            AES_ofb128_encrypt((unsigned char *) data, out, data_len, &aes_key, (unsigned char *) iv, &num);
        }
        else if (strcmp(config->encryption_mode, "cfb") == 0) {

            printf("with cfb\n");
            /* set where on the 128 bit encrypted block to begin encryption*/
            int num = 0;
            AES_cfb8_encrypt((unsigned char *) data, out, data_len, &aes_key, (unsigned char *) iv,  &num, AES_DECRYPT);
        }

        printf("OUT: %s\n", out);
        free(out);
    }
    return 0;
}
