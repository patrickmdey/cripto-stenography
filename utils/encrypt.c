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

static unsigned char * padding(unsigned char *in, int *inl, size_t blocksize);
int saveEncryptedData(unsigned char *out, int len, unsigned char *where);
void mostrarKey(unsigned char key[]);

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
        unsigned char key[MAX_AES_KEY], iv[MAX_AES_KEY];

        AES_KEY aes_key;
        int i;
        
        unsigned char * inPad = padding((unsigned char *)concat, &concat_size, AES_BLOCK_SIZE);
        unsigned char * out = calloc(concat_size + AES_BLOCK_SIZE, sizeof(char));

        if (strcmp(config->encryption_algo, "aes128") == 0) {
            // printf("Using aes128 ");
            i = EVP_BytesToKey(EVP_aes_128_cbc(), EVP_sha256(), NULL, (unsigned char *) config->password, strlen(config->password), 10, key, iv);
            if (i != 16) {
                printf("Key size is %d bits - should be 128 bits\n", i);
                return -1;
            }
            
            AES_set_encrypt_key(key, 16 * BITS, &aes_key);

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
            /* set where on the 128 bit encrypted block to begin encryption*/
            int num = 0;
            AES_ofb128_encrypt((unsigned char *) inPad, out, concat_size, &aes_key, (unsigned char *) iv, &num);
        }
        else if (strcmp(config->encryption_mode, "cfb") == 0) {

            printf("with cfb\n");
            /* set where on the 128 bit encrypted block to begin encryption*/
            int num = 0;
            AES_cfb8_encrypt((unsigned char *) inPad, out, concat_size, &aes_key, (unsigned char *) iv,  &num, AES_ENCRYPT);
        }

        printf("%s", out);
        free(inPad);
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
    *inl += pad;
    return (inPad);
}

void mostrarKey(unsigned char key[])
{
    int i;
    for (i = 0; i < 16; i++)
    {
        printf("%0x", key[i]);
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
