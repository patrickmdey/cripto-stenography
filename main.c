#include <parse_options.h>
#include <fcntl.h>
#include <decrypt.h>
#include <encrypt.h>
#include <extract.h>
#include <steg.h>

#define BUFF_INC 1024

int main(int argc, char *argv[])
{
    stegobmp_configuration_ptr config = parse_options(argc, argv);

    // int out_fd = open(config->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0);

    if (config->is_embed)
    {
        int in_fd = open(config->in_file, O_RDONLY);
        if (in_fd == -1) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
        uint32_t read_size = 0;
        char * data = read_from_file(in_fd, &read_size);
        data[read_size] = 0;
        // embed in file
        if (config->password == NULL) {
            // Solo steg, no encripto
            printf("Steg only\n");

            steg(config, data, read_size, NULL);

            free(data);
        } else {
            // steg and encrypt
            if (config->encryption_algo == NULL)
                config->encryption_algo = "aes128";

            if (config->encryption_mode == NULL)
                config->encryption_mode = "cbc";

            uint32_t cipher_length = 0;

            char * cipher = encrypt(config, data, read_size, &cipher_length, ENCRYPTION);
            steg(config, cipher, cipher_length, ".txt"); // TODO: parsear extension

            free(cipher);
            printf("Steg and encrypt\n"); //TODO logs despues
        }
    } else {
        // extract from file
        printf("Extract\n");
        if (config->password == NULL)
        {
            // Solo steg, no encripto
            printf("Extract only\n");
            extract(config);
        }
        else
        {
            // steg and decrypt
            if (config->encryption_algo == NULL)
                config->encryption_algo = "aes128";

            if (config->encryption_mode == NULL)
                config->encryption_mode = "cbc";

            extract(config);
            int in_fd = open(config->carrier_file, O_RDONLY);
            printf("%s\n", config->carrier_file);
            // READ from file descriptor until EOF
            char *file_data = malloc(sizeof(char) * BUFF_INC);
            int read_bytes;
            int total_read_chars = 0;
            int buff_len = BUFF_INC;

            while ((read_bytes = read(in_fd, file_data, BUFF_INC)) > 0)
            {
                buff_len += BUFF_INC;
                file_data = realloc(file_data, buff_len);
                if (file_data == NULL)
                {
                    printf("Failed allocating memory\n");
                    exit(1);
                }

                total_read_chars += read_bytes;
            }

            if (read_bytes == -1)
            {
                printf("ERROR reading\n");
                free(file_data);
                exit(0);
            }

            free(file_data);

            uint32_t cipher_length = 0;
            char * cipher = encrypt(config, file_data, total_read_chars, &cipher_length, DECRYPTION);

            printf("Extract and decrypt\n");
        }
    }

    free(config);
    printf("Done\n");

    return 0;
}
