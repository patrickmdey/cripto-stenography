#include <parse_options.h>

#define HAS_VALID_ARG(k) ((k) == 'i' || (k) == 'o' || (k) == 's' || (k) == 'p' || (k) == 'a' || (k) == 'm' || (k) == 'k')
enum ERROR_CODES { STATUS_SUCCESS, STATUS_ERROR };

void print_stegobmp_version(int argc) {
    if(argc == 2) {
        printf("STEGOBMP Version: %s\n", VERSION_NUMBER);
        exit(STATUS_SUCCESS);
    }
    fprintf(stderr, "Invalid use of -v option.\n");
    exit(STATUS_ERROR);
}

void print_usage() {
    printf("./stegobmp [ args.. ] \n"
           "Args: \n"
           "\t-h Prints help\n"
           "\t-embed Specifies that information is going to be hided\n"
           "\t-extract Specifies that information is going to be extracted\n"
           "\t-in [FILE] File to be hidden\n"
           "\t-p [BITMAP FILE] Carrier file in .bmp format \n"
           "\t-out [BITMAP FILE] Resultant file in .bmp format \n"
           "\t-steg [ALGORITHM] Selects the stenographic algorithm\n"
           "\t\tOPTIONS: LSB1, LSB4, LSBI\n"
           "\t-a [ALGORITHM] Block Cipher Algorithm\n"
           "\t\tOPTIONS: aes128, aes192, aes256, des\n"
           "\t-m [MODE] Block Cipher Algorithm mode\n"
           "\t\tOPTIONS: ecb, cfb, ofb, cbc\n"
           "\t-pass [PASSWORD] Encryption password\n"
           "\t-v Prints out the STEGOBMP version\n"
           );
}
void print_help() {
    printf("\n-------------------------- HELP --------------------------\n");
    print_usage();
    exit(STATUS_SUCCESS);
}

stegobmp_configuration_ptr init_stegobmp_config() {
    stegobmp_configuration_ptr stegobmp_config    = malloc(sizeof(stegobmp_configuration));
    if(stegobmp_config == NULL)
        log(FATAL, "%s", "Error while allocating memory for proxy configuration.\n");

    stegobmp_config->in_file            = NULL;
    stegobmp_config->out_file           = NULL;
    stegobmp_config->carrier_file       = NULL;
    stegobmp_config->steg_algo          = NULL;
    stegobmp_config->encryption_algo    = NULL;
    stegobmp_config->encryption_mode    = NULL;
    stegobmp_config->password           = NULL;
    stegobmp_config->is_embed           = 0;

    return stegobmp_config;
}

stegobmp_configuration_ptr parse_options(int argc, char *argv[]) {
    stegobmp_configuration_ptr stegobmp_config = init_stegobmp_config();
    int option;

    int option_index = 0;
    static struct option long_options[] = {
            {"embed",   no_argument,        0,  'e' },
            {"extract", no_argument,        0,  'x' },
            {"in",      required_argument,  0,  'i' },
            {"out",     required_argument,  0,  'o' },
            {"steg",    required_argument,  0,  's' },
            {"pass",    required_argument,  0,  'k' },
            {0, 0, 0, 0 }
    };
    //hp:a:m:vexi:o:s:k:
    while((option = getopt_long_only(argc, argv, "hp:a:m:v", long_options, &option_index)) != -1) {
        switch (option) {
        case 'h':
            print_help();
            break;
        case 'e':
            stegobmp_config->is_embed = 1;
            break;
        case 'x':
            stegobmp_config->is_embed = 0;
            break;
        case 'i':
            stegobmp_config->in_file = optarg;
            break;
        case 'o':
            stegobmp_config->out_file = optarg;
            break;
        case 'p':
            stegobmp_config->carrier_file = optarg;
            break;
        case 's':
            stegobmp_config->steg_algo = optarg;
            break;
        case 'a':
            stegobmp_config->encryption_algo = optarg;
            break;
        case 'm':
            stegobmp_config->encryption_mode = optarg;
            break;
        case 'k':
            stegobmp_config->password = optarg;
            break;
        case 'v':
            print_stegobmp_version(argc);
            break;
        case '?':
            break;
        default:
            fprintf(stderr, "Invalid options, use -h to print help\n");
            exit(STATUS_ERROR);
            break;
        }
    }

    return stegobmp_config;
}
