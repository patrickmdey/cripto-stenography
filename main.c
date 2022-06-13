#include <parse_options.h>
#include <fcntl.h>
#include <utils.h>
#include <decrypt.h>
#include <encrypt.h>
#include <extract.h>
#include <steg.h>
#include <utils.h>

#define BUFF_INC 1024


int main(int argc, char * argv[]) {
    stegobmp_configuration_ptr config = parse_options(argc, argv);

    if (config->is_embed) {
        int in_fd = open(config->in_file, O_RDONLY);
        if (in_fd == -1) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
        uint32_t read_size = 0;
        char * data = read_from_file(in_fd, &read_size);
        // data[read_size] = 0;
        // printf("%s\n", data);
        // embed in file
        if (config->password == NULL) {
            // Solo steg, no encripto
            printf("Steg only\n");

            char * extension = get_extension(config->in_file);
            printf("Extension: %s\n", extension);
            steg(config, data, read_size, extension);// TODO: parsear extension
            free(extension);
        }
        else {
            // steg and encrypt
            if (config->encryption_algo == NULL)
                config->encryption_algo = "aes128";

            if (config->encryption_mode == NULL)
                config->encryption_mode = "cbc";

            uint32_t cipher_length = 0;

            char * cipher = encrypt(config, data, read_size, &cipher_length, ENCRYPTION);
            cipher[cipher_length] = 0;
            printf("Cipher length: %d\n", cipher_length);
            printf("Cipher: %s\n", cipher);

            steg(config, cipher, cipher_length, NULL);
            free(cipher);

            printf("Steg and encrypt\n"); //TODO logs despues
        }

        free(data);

    }
    else {
        // extract from file
        printf("Extract\n");
        int in_fd = open(config->carrier_file, O_RDONLY);
        if (in_fd == -1) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }

        uint32_t read_size = 0;
        char * data = read_from_file(in_fd, &read_size);
        data[read_size] = 0;

        uint32_t output_data_length = 0;
        char * output_data = NULL;

        if (config->password == NULL) {
            // Solo steg, no encripto
            printf("Extract only\n");
            // uint32_t hidden_size = 0;
            output_data = steg_extract(config, data, read_size, &output_data_length);

            //free(hidden_data);
        } else {
            // steg and decrypt
            if (config->encryption_algo == NULL)
                config->encryption_algo = "aes128";

            if (config->encryption_mode == NULL)
                config->encryption_mode = "cbc";

            // extract(config);
            uint32_t hidden_size = 0;//, cipher_length = 0;
            char * hidden_data = steg_extract(config, data, read_size, &hidden_size);
            
            output_data = encrypt(config, hidden_data, hidden_size, &output_data_length, DECRYPTION);
            // printf("Salida size: %d\n", cipher_length);
            // // cipher[cipher_length] = 0;
            // printf("SALIDA: %s\n", cipher + 4);
            // free(cipher);
            // printf("Extract and decrypt\n");
            free(hidden_data);
        }

        int out_fd = open(config->out_file, O_WRONLY | O_CREAT, 0644);
        if (out_fd == -1) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }

        // TODO: ver que hacemo, el + 4 es para ignorar el tamano, 
        // falta sacarle la extension del final tmb pero necesito tenerla o al menos la longitud
        write_to_file(out_fd, output_data + 4, output_data_length - 4);

        close(out_fd);

        free(output_data);
        free(data);
    }

    free(config);
    printf("Done\n");
    return 0;
}
