#include <parse_options.h>
#include <fcntl.h>
#include <utils.h>
#include <encrypt.h>
#include <steg.h>
#include <utils.h>
#include <logger.h>

#define BUFF_INC 1024

int check_config(stegobmp_configuration_ptr config) {
    return config->steg_algo != NULL && config->out_file != NULL && config->carrier_file != NULL &&
            ((config->is_embed && config->in_file != NULL) || !config->is_embed);
}

int main(int argc, char * argv[]) {
    stegobmp_configuration_ptr config = parse_options(argc, argv);
    uint8_t is_encryption = config->password != NULL ? ENCRYPTION : DECRYPTION;

    if (!check_config(config))
        log(FATAL, "Invalid arguments, please use help: ./stegobmp -help %s", "");

    if (config->is_embed) {
        log(INFO, "Preparing to embed...%s", "");
        int in_fd = open(config->in_file, O_RDONLY);
        if (in_fd == -1)
            log(FATAL, "Could not open input file %s", config->in_file);

        uint32_t read_size = 0;
        char * data = read_from_file(in_fd, &read_size);

        char * extension = NULL;

        if (!is_encryption) {
            extension = get_extension(config->in_file);
        }
        else {
            log(INFO, "Encrypting file %s", config->in_file);

            if (config->encryption_algo == NULL)
                config->encryption_algo = "aes128";

            if (config->encryption_mode == NULL)
                config->encryption_mode = "cbc";

            uint32_t cipher_length = 0;
            char * aux;
            aux = encrypt(config, data, read_size, &cipher_length, ENCRYPTION);

            memcpy(data, aux, cipher_length);
            free(aux);
            read_size = cipher_length;
        }
        steg(config, data, read_size, extension);

        free(data);
        if (extension != NULL)
            free(extension);
        log(INFO, "Embedding finished%s", "");
    }
    else {
        log(INFO, "Preparing to extract...%s", "");

        int in_fd = open(config->carrier_file, O_RDONLY);
        if (in_fd == -1) {
            log(FATAL, "Could not open carrier file %s", config->carrier_file);
        }

        uint32_t read_size = 0;
        char * data = read_from_file(in_fd, &read_size);

        uint32_t hidden_data_length = 0;
        char * hidden_data = NULL;

        hidden_data = steg_extract(config, data, read_size, &hidden_data_length, is_encryption);

        char * extension;
        if (is_encryption) {
            log(INFO, "Decrypting hidden data%s", "");
            if (config->encryption_algo == NULL)
                config->encryption_algo = "aes128";

            if (config->encryption_mode == NULL)
                config->encryption_mode = "cbc";

            uint32_t plain_size = 0;
            char * plain_data = encrypt(config, hidden_data, hidden_data_length, &plain_size, DECRYPTION);

            memcpy(hidden_data, plain_data + sizeof(uint32_t), plain_size);
            extension = get_extension(plain_data + plain_size + sizeof(uint32_t));

            free(plain_data);
            hidden_data_length = plain_size;
        }
        else {
            extension = get_extension(hidden_data + hidden_data_length);
        }

        log(INFO, "Hidden file has extension: %s", extension);

        char file_name[strlen(config->out_file) + strlen(extension) + 1];
        strcpy(file_name, config->out_file);
        strcat(file_name, extension);

        free(extension);

        int out_fd = open(file_name, O_WRONLY | O_CREAT, 0644);
        if (out_fd == -1)
            log(FATAL, "Could not open output file %s", config->out_file);

        write_to_file(out_fd, hidden_data, hidden_data_length);

        close(out_fd);

        free(hidden_data);
        free(data);
    }

    free(config);
    return 0;
}
