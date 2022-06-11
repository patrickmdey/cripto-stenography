#include <steg.h>

#define BUFF_INC 1024
#define NO_COMPRESSION 0
#define HEADER_SIZE 54
#define LEAST_SIGINIFICANT 00000001
#define BITS_PER_PIXEL 24
#define BMP_TYPE 0x4d42

#define LSB1_MASK 0x01

#pragma pack(push, 1)

typedef struct BMPHeader
{                              // Total: 54 bytes
    uint16_t type;             // Magic identifier: 0x4d42
    uint32_t size;             // File size in bytes
    uint16_t reserved1;        // Not used
    uint16_t reserved2;        // Not used
    uint32_t offset;           // Offset to image data in bytes from beginning of file (54 bytes)
    uint32_t dib_header_size;  // DIB Header size in bytes (40 bytes)
    int32_t width_px;          // Width of the image
    int32_t height_px;         // Height of image
    uint16_t num_planes;       // Number of color planes
    uint16_t bits_per_pixel;   // Bits per pixel
    uint32_t compression;      // Compression type
    uint32_t image_size_bytes; // Image size in bytes
    int32_t x_resolution_ppm;  // Pixels per meter
    int32_t y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;       // Number of colors
    uint32_t important_colors; // Important colors
} BMPHeader;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct BMPImage
{
    BMPHeader header;
    unsigned char *data;
} BMPImage;

#pragma pack(pop)

static uint8_t is_invalid_bmp(BMPImage_ptr bmp_image);
static int lsb1(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd);
static int lsb4(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd);
static int write_to_file(int fd, const char * buff, int bytes);

int steg(stegobmp_configuration_ptr config, char * embed_data, uint32_t embed_data_length, char * extension) {
    int carrier_fd = open(config->carrier_file, O_RDONLY);
    if (carrier_fd == -1) {
        perror("Error opening carrier file");
        exit(EXIT_FAILURE);
    }

    char *buff = calloc(BUFF_INC, sizeof(char));
    uint32_t total_read_chars = 0;
    int read_bytes;
    int bytes_to_read = HEADER_SIZE;

    BMPImage_ptr bmp_image = calloc(1, sizeof(BMPImage));

    int offset = 0;
    while (bytes_to_read > 0 && (read_bytes = read(carrier_fd, buff, bytes_to_read)) > 0)
    {
        if (read_bytes == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        bytes_to_read -= read_bytes;
        memcpy(&bmp_image->header + offset, buff, read_bytes);
        offset += read_bytes;
    }

    if (is_invalid_bmp(bmp_image))
    {
        printf("%d\n", bmp_image->header.compression);
        printf("%x\n", bmp_image->header.type);
        printf("%x\n", bmp_image->header.bits_per_pixel);
        free(buff);
        printf("Invalid bmp\n");
        exit(0);
    }

    bmp_image->data = (unsigned char *) read_from_file(carrier_fd, &total_read_chars);

    printf("%s\n", config->out_file);
    int out_fd = open(config->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("open");
        close(out_fd);
        exit(EXIT_FAILURE);
    }


    uint32_t hidden_size = embed_data_length + sizeof(uint32_t) + 1;
    if (extension != NULL)
        hidden_size += strlen(extension) + 1;

    char hidden_data[hidden_size];
    memset(hidden_data, 0, hidden_size);

    uint32_t big_endian_size = htobe32(embed_data_length);

    memcpy(hidden_data, &big_endian_size, sizeof(uint32_t));
    strcat(hidden_data + sizeof(uint32_t), embed_data);

    if (extension != NULL) {
        strcat(hidden_data + sizeof(uint32_t) + embed_data_length, ".");
        strcat(hidden_data + sizeof(uint32_t) + embed_data_length + 1, extension);
    }

    if (strcmp(config->steg_algo, "LSB1") == 0) {
        lsb1(bmp_image, hidden_data, hidden_size, out_fd);
    }
    else if (strcmp(config->steg_algo, "LSB4") == 0) {
        lsb4(bmp_image, hidden_data, hidden_size, out_fd);
    }
    else if (strcmp(config->steg_algo, "LSBI") == 0) {
    }
    else {
        close(out_fd);
        printf("Invalid Steg method\n");
        return -1;
    }

    free(buff);
    free(bmp_image->data);
    free(bmp_image);
    return 0;
}

int steg_extract(stegobmp_configuration_ptr config, char * extract_data, uint32_t extract_data_length) {
    BMPImage_ptr bmp_image = calloc(1, sizeof(BMPImage));
    memcpy(&bmp_image->header, extract_data, HEADER_SIZE);
    bmp_image->data = calloc(extract_data_length - sizeof(BMPHeader), sizeof(unsigned char));
    memcpy(bmp_image->data, extract_data + HEADER_SIZE, extract_data_length - sizeof(BMPHeader));

    if (strcmp(config->steg_algo, "LSB1") == 0) {
        
    }
    else if (strcmp(config->steg_algo, "LSB4") == 0){

    }
    else if (strcmp(config->steg_algo, "LSBI") == 0){

    }

    free(bmp_image->data);
    free(bmp_image);
    return 0;
}

static uint8_t is_invalid_bmp(BMPImage_ptr bmp_image) {
    return bmp_image->header.type != BMP_TYPE || bmp_image->header.compression != NO_COMPRESSION ||
            bmp_image->header.bits_per_pixel != BITS_PER_PIXEL;
}

static int lsb1(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd) {
    if (embed_data_length * 8 > bmp_image->header.image_size_bytes) {
        printf("Aflojale con el espacio rey\n");
        exit(0);
    }

    // pixels per row * amount of rows // c/pixel = 3bytes
    // int image_size = bmp_image->header.width_px * bmp_image->header.height_px;
    char *buff = calloc(bmp_image->header.image_size_bytes, sizeof(char));
    memcpy(buff, bmp_image->data, bmp_image->header.image_size_bytes);
    int curr_byte = 0;

    for (uint32_t i = 0; i < embed_data_length; i++) {
        for (uint32_t j = 0; j < 8; j++, curr_byte++) {
            uint8_t bit = (embed_data[i] >> (7 - j)) & 1;
            buff[curr_byte] = (bmp_image->data[curr_byte] & 0xFE) | (bit);
            // sifteado = 00000001
            // blue= 10010101
            // OxE = 11111110
            //     = 10010100
        }
    }

    int ret = write_to_file(out_fd, (char *)&bmp_image->header, HEADER_SIZE);
    if (ret == -1) {
        printf("Failed writing to file\n");
        exit(1);
    }
    ret = write_to_file(out_fd, buff, bmp_image->header.image_size_bytes);
    if (ret == -1) {
        printf("Failed writing to file\n");
        exit(1);
    }

    close(out_fd);
    free(buff);
    return 0;
}


static int lsb4(BMPImage_ptr bmp_image, char *embed_data, uint32_t embed_data_length, int out_fd) {
    printf("LSB4\n");
    if (embed_data_length * 2 > bmp_image->header.image_size_bytes) {
        printf("Aflojale con el espacio rey\n");
        exit(0);
    }

    char *buff = calloc(bmp_image->header.image_size_bytes, sizeof(char));
    memcpy(buff, bmp_image->data, bmp_image->header.image_size_bytes);
    int curr_byte = 0;

    for (uint32_t i = 0; i < embed_data_length; i++) {
        for (uint32_t j = 0; j < 2; j++, curr_byte++) {
            uint8_t bits = (embed_data[i] >> (j * 4)) & 0xF; // TODO: No esta al reves?
            buff[curr_byte] = (bmp_image->data[curr_byte] & 0xF0) | (bits);
        }
    }

    int ret = write_to_file(out_fd, (char *)&bmp_image->header, HEADER_SIZE);
    if (ret == -1) {
        printf("Failed writing to file\n");
        exit(1);
    }
    ret = write_to_file(out_fd, buff, bmp_image->header.image_size_bytes);
    if (ret == -1) {
        printf("Failed writing to file\n");
        exit(1);
    }

    close(out_fd);
    free(buff);
    return 0;
}

static int write_to_file(int fd, const char *buff, int bytes) {
    int bytes_written = 0;
    while (bytes_written < bytes)
    {
        int bytes_to_write = write(fd, buff + bytes_written, bytes - bytes_written);
        if (bytes_to_write == -1)
        {
            perror("write");
            return -1;
        }
        bytes_written += bytes_to_write;
    }
    return bytes_written;
}
