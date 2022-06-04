#include <parse_options.h>

int main(int argc, char * argv[]) {
    stegobmp_configuration_ptr config = parse_options(argc, argv);
    return 0;
}
