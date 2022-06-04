// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <parse_options.h>


#define HAS_VALID_ARG(k) ((k) == 'in' || (k) == 'out' || (k) == 'steg' || (k) == 'p' || (k) == 'a' || (k) == 'm' || (k) == 'pass' || (k) == 't')
enum ERROR_CODES { STATUS_SUCCESS, STATUS_ERROR };

void print_proxy_version(int argc) {
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
           "\t-embed Sepcifies that information is going to be hided\n"
           "\t-extract Sepcifies that information is going to be extracted\n"
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
    if(stegobmp_config == NULL) {
        // log(FATAL, "%s", "Error while allocating memory for proxy configuration.\n");
        exit(STATUS_ERROR);
    }
    stegobmp_config->in_file            = NULL;
    stegobmp_config->out_file           = NULL;
    stegobmp_config->carrier_file       = NULL;
    stegobmp_config->steg_algo          = NULL;
    stegobmp_config->encryption_algo    = NULL;
    stegobmp_config->encryption_mode    = NULL;
    stegobmp_config->password           = NULL;
    stegobmp_config->isEmbed            = -1;

    return stegobmp_config;
}

stegobmp_configuration_ptr parse_options(int argc, char *argv[]) {
    stegobmp_configuration_ptr stegobmp_config = init_stegobmp_config();
    int option;

    while((option = getopt(argc, argv, "hembedextractin:out:p:steg:a:m:pass:v")) != -1) {
        switch (option) {
        case 'h':
            print_help();
            break;
        case 'embed':
            stegobmp_config->isEmbed = 1;
            break;
        case 'extract':
            stegobmp_config->isEmbed = 0;
            break;
        case 'in':
            stegobmp_config->in_file = optarg;
            break;
        case 'out':
            stegobmp_config->out_file = optarg;
            break;
        case 'p':
            stegobmp_config->out_file = optarg;
            break;
        case 'steg':
            stegobmp_config->steg_algo = optarg;
            break;
        case 'a':
            stegobmp_config->encryption_algo = optarg;
            break;
        case 'm':
            stegobmp_config->encryption_mode = optarg;
            break;
        case 'pass':
            stegobmp_config->password = optarg;
            break;
        case 'v':
            print_proxy_version(argc);
            break;
        case '?':
            if(HAS_VALID_ARG(optopt)) {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            } else if(isprint(optopt)) {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            } else {
                fprintf (stderr,"Unknown option character `\\x%x'.\n", optopt);
            }
            break;

        default:
            fprintf(stderr, "Invalid options, use -h to print help\n");
            exit(STATUS_ERROR);
            break;
        }
    }

    if(argc - optind != 1) {
        fprintf(stderr, "Invalid args, please use: stegbmp [ARGS]\n");
        exit(STATUS_ERROR);
    }

    int is_invalid_arg = 0;
    for (int index = optind; index < argc-1; index++) {
        printf ("Invalid argument %s\n", argv[index]);
        is_invalid_arg = 1;
    }

    if(is_invalid_arg)
        exit(STATUS_SUCCESS);

    // proxy_config->origin_server_address = argv[optind];
    return stegobmp_config;
}
