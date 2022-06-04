CFLAGS = -c -g --std=c11 -fsanitize=address -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200809L -lcrypto -lssl
LFLAGS = -g --std=c11 -fsanitize=address -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200809L -lcrypto -lssl
