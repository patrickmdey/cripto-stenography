#include <encrypt.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFF_INC 1024

int encrypt(stegobmp_configuration_ptr config) {
    int in_fd = open(config->in_file, O_RDONLY);
    // READ from file descriptor until EOF
    char * buff = malloc(sizeof(char) * BUFF_INC);
    int read_bytes;
    int total_read_chars = 0;
    int buff_len = BUFF_INC;

    while ((read_bytes = read(in_fd, buff, BUFF_INC)) > 0) {
        // printf("%d\n", read_bytes);
        buff_len += BUFF_INC;
        buff = realloc(buff, buff_len);
        if (buff == NULL) {
            printf("Failed allocating memory\n");
            exit(1);
        }
        total_read_chars += read_bytes;
    }
    printf("%d\n", read_bytes);

    
    if (read_bytes == -1) {
        printf("ERROR\n");
        free(buff);
        exit(0);
    }

    
    buff[total_read_chars] = '\0';


    printf("%s\n", buff);

    unsigned char key[32], iv[32];

    AES_KEY aes_key;

    int i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), NULL, (unsigned char *) config->password, strlen(config->password), 16, key, iv);
    if (i != 32) {
        printf("Key size is %d bits - should be 256 bits\n", i);
        return -1;
    }
    
    unsigned char * out = malloc(sizeof(char) * 256); 
    AES_set_encrypt_key(key, 256, &aes_key);
    AES_cbc_encrypt((unsigned char *) buff, out, total_read_chars, &aes_key, iv, AES_ENCRYPT);

    out[strlen("PRUEBA")] = '\0';
    printf("%s\n", out);
    free(out);
    free(buff);
    return 0;
}
