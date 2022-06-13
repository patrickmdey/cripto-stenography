
#include <utils.h>
 
// Function to swap two numbers
static void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}
 
// Function to reverse `buffer[i…j]`
static char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}
 
// Iterative function to implement `itoa()` function in C
char* itoa(int value, char* buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }
 
    // consider the absolute value of the number
    int n = abs(value);
 
    int i = 0;
    while (n)
    {
        int r = n % base;
 
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
 
        n = n / base;
    }
 
    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }
 
    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }
 
    buffer[i] = '\0'; // null terminate string
 
    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}

char * read_from_file(int fd, uint32_t * read_chars) {
    char *buff = calloc(BUFF_INC, sizeof(char));
    int total_read_chars = 0;
    int read_bytes;
    int buff_len = BUFF_INC;
        
    while ((read_bytes = read(fd, buff+total_read_chars, BUFF_INC)) > 0) {
        buff_len += BUFF_INC;
        buff = realloc(buff, buff_len);
        if (buff == NULL) {
            printf("Failed allocating memory\n");
            exit(1);
        }

        total_read_chars += read_bytes;
    }

    *read_chars = total_read_chars;
    return buff;
}

int write_to_file(int fd, const char * buff, int bytes) {
    int bytes_written = 0;
    while (bytes_written < bytes) {
        int bytes_to_write = write(fd, buff + bytes_written, bytes - bytes_written);
        if (bytes_to_write == -1) {
            perror("write");
            return -1;
        }
        bytes_written += bytes_to_write;
    }
    return bytes_written;
}

char * get_extension(char * string){
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