
#include <utils.h>

char * read_from_file(int fd, uint32_t * read_chars) {
    char * buff = calloc(BUFF_INC, sizeof(char));
    if (buff == NULL)
        log(FATAL, "Failed to allocate memory for buffer%s", "");

    int total_read_chars = 0;
    int read_bytes;
    int buff_len = BUFF_INC;

    while ((read_bytes = read(fd, buff + total_read_chars, BUFF_INC)) > 0) {
        buff_len += BUFF_INC;
        buff = realloc(buff, buff_len);
        if (buff == NULL)
            log(FATAL, "Failed to allocate memory for buffer%s", "");

        total_read_chars += read_bytes;
    }

    if (read_bytes == -1)
        log(FATAL, "Failed to read from file%s", "");

    *read_chars = total_read_chars;
    return buff;
}

int write_to_file(int fd, const char * buff, int bytes) {
    int bytes_written = 0;
    while (bytes_written < bytes) {
        int bytes_to_write = write(fd, buff + bytes_written, bytes - bytes_written);
        if (bytes_to_write == -1)
            log(FATAL, "Failed to write to file%s", "");

        bytes_written += bytes_to_write;
    }
    return bytes_written;
}

char * get_extension(char * string) {
    char * extension;
    char * token;
    token = strtok(string, ".");

    while (token != NULL) {
        extension = token;
        token = strtok(NULL, ".");
    }
    char * extension_copy = calloc(strlen(extension) + 2, sizeof(char));
    strcpy(extension_copy, ".");
    strcat(extension_copy, extension);
    return extension_copy;
}
