#include <encrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFF_INC 1024


int encrypt(stegobmp_configuration_ptr config) {
    int in_fd = open(config->in_file, O_RDONLY);
    // READ from file descriptor until EOF
    char * file_data= malloc(sizeof(char) * BUFF_INC);
    int read_bytes;
    int total_read_chars = 0;
    int buff_len = BUFF_INC;

    while ((read_bytes = read(in_fd, file_data, BUFF_INC)) > 0) {
        // printf("%d\n", read_bytes);
        buff_len += BUFF_INC;
        file_data= realloc(file_data, buff_len);
        if (file_data== NULL) {
            printf("Failed allocating memory\n");
            exit(1);
        }
        total_read_chars += read_bytes;
    }
    printf("%d\n", read_bytes);

    
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
    printf("%s\n", extension);
    extension = strtok(NULL, ".");

    printf("%s\n", extension);

    int concat_size = 4 + total_read_chars + strlen(extension+1);
    char *concat = calloc(concat_size + 1, sizeof(char));

    strcat(concat, file_size);
    strcat(concat, file_data);
    strcat(concat, ".");
    strcat(concat, extension);
    printf("%s\n",concat);
    
    free(file_data);

    unsigned char key[32], iv[32];

    AES_KEY aes_key;

    int i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, iv);
    if (i != 32) {
        printf("Key size is %d bits - should be 256 bits\n", i);
        return -1;
    }
    
    unsigned char * out = malloc(sizeof(char) * 256);
    AES_set_encrypt_key(key, 256, &aes_key);
    AES_cbc_encrypt((unsigned char *) concat, out, total_read_chars, &aes_key, iv, AES_ENCRYPT);

    out[strlen("PRUEBA")] = '\0';
    printf("%s\n", out);
    free(out);
    free(concat);
    return 0;
}
