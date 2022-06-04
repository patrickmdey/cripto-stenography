#include <parse_options.h>
#include <fcntl.h>
#include <decrypt.h>
#include <encrypt.h>
#include <extract.h>
#include <steg.h>

int main(int argc, char * argv[]) {
    stegobmp_configuration_ptr config = parse_options(argc, argv);

    // int out_fd = open(config->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0);

    if (config->is_embed) {
        // embed in file
        if (config->password == NULL) {
            // Solo steg, no encripto
            printf("Steg only\n");
            steg(config);
        } else {
            // steg and encrypt
            if (config->encryption_algo == NULL) 
                config->encryption_algo = "aes128";
            
            if (config->encryption_mode == NULL) 
                config->encryption_mode = "CBC";
                
            encrypt(config);
            steg(config);
            printf("Steg and encrypt\n");
        }
    } else {
        // extract from file
        printf("Extract\n");
        if (config->password == NULL) {
            // Solo steg, no encripto
            printf("Extract only\n");
            extract(config);
        } else {
            // steg and decrypt
            if (config->encryption_algo == NULL) 
                config->encryption_algo = "aes128";
            
            if (config->encryption_mode == NULL) 
                config->encryption_mode = "CBC";
                
            decrypt(config);
            extract(config);
            printf("Extract and decrypt\n");
        }
    }

    free(config);

    return 0;
}
