#include <steg.h>

#define BUFF_INC            1024
#define NO_COMPRESSION      0
#define HEADER_SIZE         54
#define LEAST_SIGINIFICANT  00000001
#define BITS_PER_PIXEL      24
#define BMP_TYPE            0x4d42

#define LSB1_MASK 0xFE
#define LSB4_MASK 0xF0
#define PATTERN_MASK 0x06 // 0000 0110

#pragma pack(push, 1)

typedef struct BMPHeader {          // Total: 54 bytes
    uint16_t    type;               // Magic identifier: 0x4d42
    uint32_t    size;               // File size in bytes
    uint16_t    reserved1;          // Not used
    uint16_t    reserved2;          // Not used
    uint32_t    offset;             // Offset to image data in bytes from beginning of file (54 bytes)
    uint32_t    dib_header_size;    // DIB Header size in bytes (40 bytes)
    int32_t     width_px;           // Width of the image
    int32_t     height_px;          // Height of image
    uint16_t    num_planes;         // Number of color planes
    uint16_t    bits_per_pixel;     // Bits per pixel
    uint32_t    compression;        // Compression type
    uint32_t    image_size_bytes;   // Image size in bytes
    int32_t     x_resolution_ppm;   // Pixels per meter
    int32_t     y_resolution_ppm;   // Pixels per meter
    uint32_t    num_colors;         // Number of colors
    uint32_t    important_colors;   // Important colors
} BMPHeader;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct BMPImage {
    BMPHeader       header;
    unsigned char * data;
} BMPImage;

#pragma pack(pop)

static uint8_t  is_invalid_bmp(BMPImage_ptr bmp_image);
static char * lsb1(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd, uint8_t writes_to_file);
static int lsb4(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd);
static int lsbi(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd);
static char * lsb1_extract(BMPImage_ptr bmp_image, uint32_t * hidden_size, uint8_t is_encryption);
static char * lsb4_extract(BMPImage_ptr bmp_image, uint32_t * hidden_size, uint8_t is_encryption);
static char * lsbi_extract(BMPImage_ptr bmp_image, uint32_t * hidden_size, uint8_t is_encryption);


int steg(stegobmp_configuration_ptr config, char * embed_data, uint32_t embed_data_length, char * extension) {
    int carrier_fd = open(config->carrier_file, O_RDONLY);
    if (carrier_fd == -1)
        log(FATAL, "Could not open carrier file: %s", config->carrier_file);

    char * buff = calloc(BUFF_INC, sizeof(char));
    if (buff == NULL)
        log(FATAL, "Could not allocate memory for buffer%s", "");

    uint32_t total_read_chars = 0;
    int read_bytes;
    int bytes_to_read = HEADER_SIZE;

    BMPImage_ptr bmp_image = calloc(1, sizeof(BMPImage));
    if (bmp_image == NULL)
        log(FATAL, "Could not allocate memory for BMPImage%s", "");

    int offset = 0;
    while (bytes_to_read > 0 && (read_bytes = read(carrier_fd, buff, bytes_to_read)) > 0) {
        bytes_to_read -= read_bytes;
        memcpy(&bmp_image->header + offset, buff, read_bytes);
        offset += read_bytes;
    }

    if (read_bytes == -1)
        log(FATAL, "Could not read from carrier file: %s", config->carrier_file);

    if (is_invalid_bmp(bmp_image))
        log(FATAL, "Invalid BMP file: %s. Correct format is no compression and 24 bits per pixel", config->carrier_file);

    bmp_image->data = (unsigned char *)read_from_file(carrier_fd, &total_read_chars);

    int out_fd = open(config->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1)
        log(FATAL, "Could not open output file: %s", config->out_file);

    uint32_t hidden_size = embed_data_length + sizeof(uint32_t);
    if (extension != NULL)
        hidden_size += strlen(extension) + 1; // + 2 si hay .

    char hidden_data[hidden_size];
    memset(hidden_data, 0, hidden_size);

    uint32_t big_endian_size = htobe32(embed_data_length);

    memcpy(hidden_data, &big_endian_size, sizeof(uint32_t));
    memcpy(hidden_data + sizeof(uint32_t), embed_data, embed_data_length);

    if (extension != NULL) {
        memcpy(hidden_data + sizeof(uint32_t) + embed_data_length, extension, strlen(extension) + 1);
    }

    if (strcmp(config->steg_algo, "LSB1") == 0) {
        char * out_data = lsb1(bmp_image, hidden_data, hidden_size, out_fd, 1); // TODO: cambiar
        free(out_data);
    }
    else if (strcmp(config->steg_algo, "LSB4") == 0) {
        lsb4(bmp_image, hidden_data, hidden_size, out_fd);
    }
    else if (strcmp(config->steg_algo, "LSBI") == 0) {
        lsbi(bmp_image, hidden_data, hidden_size, out_fd);
    }
    else {
        log(FATAL, "Invalid steg algorithm: %s", config->steg_algo);
    }

    free(buff);
    free(bmp_image->data);
    free(bmp_image);
    return 0;
}

char * steg_extract(stegobmp_configuration_ptr config, char * extract_data, uint32_t extract_data_length,
    uint32_t * hidden_size, uint8_t is_encryption) {

    BMPImage_ptr bmp_image = calloc(1, sizeof(BMPImage));
    if (bmp_image == NULL)
        log(FATAL, "Could not allocate memory for BMPImage%s", "");

    memcpy(&bmp_image->header, extract_data, HEADER_SIZE);
    bmp_image->data = calloc(extract_data_length - sizeof(BMPHeader), sizeof(unsigned char));
    if (bmp_image->data == NULL)
        log(FATAL, "Could not allocate memory for BMPImage data%s", "");

    memcpy(bmp_image->data, extract_data + HEADER_SIZE, extract_data_length - sizeof(BMPHeader));

    char * hidden_data;

    if (strcmp(config->steg_algo, "LSB1") == 0) { // TODO: capaz pasar todo a un enum
        hidden_data = lsb1_extract(bmp_image, hidden_size, is_encryption);
    }
    else if (strcmp(config->steg_algo, "LSB4") == 0) {
        hidden_data = lsb4_extract(bmp_image, hidden_size, is_encryption);
    }
    else if (strcmp(config->steg_algo, "LSBI") == 0) {
        hidden_data = lsbi_extract(bmp_image, hidden_size, is_encryption);
    }

    free(bmp_image->data);
    free(bmp_image);
    return hidden_data;
}

static uint8_t is_invalid_bmp(BMPImage_ptr bmp_image) {
    return bmp_image->header.type != BMP_TYPE || bmp_image->header.compression != NO_COMPRESSION ||
        bmp_image->header.bits_per_pixel != BITS_PER_PIXEL;
}

static char * lsb1_extract(BMPImage_ptr bmp_image, uint32_t * hidden_size, uint8_t is_encryption) {
    uint8_t byte = 0;

    uint8_t size_bytes[4];

    uint32_t offset = sizeof(uint32_t) * 8;

    for (uint32_t i = 0; i < offset; i++) {
        uint8_t bit = bmp_image->data[i] & 1;
        byte = (byte << 1) | bit;
        if ((i + 1) % 8 == 0) {
            size_bytes[((i + 1) / 8) - 1] = byte;
            byte = 0;
        }
    }

    memcpy(hidden_size, size_bytes, sizeof(uint32_t));
    *hidden_size = be32toh(*hidden_size);

    char * hidden_data = calloc(*hidden_size, sizeof(char));

    for (uint32_t i = 0; i < (*hidden_size) * 8; i++) {
        uint8_t bit = bmp_image->data[i + offset] & 1;
        byte = (byte << 1) | bit;
        if ((i + 1) % 8 == 0) {
            hidden_data[((i + 1) / 8) - 1] = byte;
            byte = 0;
        }
    }

    // Si no hay encripcion, el siguiente byte deberia ser un .
    if (!is_encryption) {
        char extension[8];
        uint8_t found = 0;
        offset += (*hidden_size) * 8;

        for (uint32_t i = 0; !found; i++) {
            uint8_t bit = bmp_image->data[offset + i] & 1;
            byte = (byte << 1) | bit;
            if ((i + 1) % 8 == 0) {
                extension[((i + 1) / 8) - 1] = byte;

                if (byte == 0) {
                    found = 1;
                    hidden_data = realloc(hidden_data, *hidden_size + ((i + 1) / 8));
                    memcpy(hidden_data + *hidden_size, extension, (i + 1) / 8);
                }
                byte = 0;
            }
        }
    }


    return hidden_data;
}

static char * lsb4_extract(BMPImage_ptr bmp_image, uint32_t * hidden_size, uint8_t is_encryption) {
    uint8_t byte = 0;
    uint8_t size_bytes[4];

    uint32_t offset = sizeof(uint32_t) * 2;

    for (uint32_t i = 0; i < offset; i++) {
        uint8_t bits = bmp_image->data[i] & 0x0F;
        byte = (byte << 4) | bits;
        if ((i + 1) % 2 == 0) {
            size_bytes[((i + 1) / 2) - 1] = byte;
            byte = 0;
        }
    }

    memcpy(hidden_size, size_bytes, sizeof(uint32_t));
    *hidden_size = be32toh(*hidden_size);

    char * hidden_data = calloc(*hidden_size + 1, sizeof(char)); // + 1 para el 0

    for (uint32_t i = 0; i < *hidden_size * 2; i++) {
        uint8_t bits = bmp_image->data[i + offset] & 0x0F;
        byte = (byte << 4) | bits;
        if ((i + 1) % 2 == 0) {
            hidden_data[((i + 1) / 2) - 1] = byte;
            byte = 0;
        }
    }

    // Si no hay encripcion, el siguiente byte deberia ser un .
    if (!is_encryption) {
        char extension[8];
        uint8_t found = 0;
        offset += (*hidden_size) * 8;

        for (uint32_t i = 0; !found; i++) {
            uint8_t bit = bmp_image->data[offset + i] & 0X0F;
            byte = (byte << 4) | bit;
            if ((i + 1) % 2 == 0) {
                extension[((i + 1) / 2) - 1] = byte;

                if (byte == 0) {
                    found = 1;
                    hidden_data = realloc(hidden_data, *hidden_size + ((i + 1) / 2));
                    memcpy(hidden_data + *hidden_size, extension, (i + 1) / 2);
                }
                byte = 0;
            }
        }
    }

    return hidden_data;
}

static char * lsbi_extract(BMPImage_ptr bmp_image, uint32_t * hidden_size, uint8_t is_encryption) {
    uint8_t byte = 0;
    uint8_t size_bytes[4];

    uint8_t is_inverted[4] = { 0 };

    // 00b = 0, 01b = 1, 10b = 2, 11b = 3
    for (uint32_t i = 0; i < 4; i++)
        is_inverted[i] = bmp_image->data[i] & 1;


    // TODO: ver si va, es pq leo de der a izq en vez de izq a der
    uint8_t aux = is_inverted[1];
    is_inverted[1] = is_inverted[2];
    is_inverted[2] = aux;

    uint32_t offset = 4;

    uint8_t bit, pattern;
    for (uint32_t i = 0; i < sizeof(uint32_t) * 8; i++) {
        bit = bmp_image->data[offset + i] & 1;
        pattern = (bmp_image->data[offset + i] & PATTERN_MASK) >> 1; // 6 = 110b
        if (is_inverted[pattern])
            bit = 1 - bit;

        byte = (byte << 1) | bit;
        if ((i + 1) % 8 == 0) {
            size_bytes[((i + 1) / 8) - 1] = byte;
            byte = 0;
        }
    }

    offset += sizeof(uint32_t) * 8;

    memcpy(hidden_size, size_bytes, sizeof(uint32_t));
    *hidden_size = be32toh(*hidden_size);

    char * hidden_data = calloc(*hidden_size + 1, sizeof(char)); // TODO: ver + 1 para el 0

    for (uint32_t i = 0; i < (*hidden_size) * 8; i++) {
        bit = bmp_image->data[offset + i] & 1;

        pattern = (bmp_image->data[offset + i] & PATTERN_MASK) >> 1; // 6 = 110b
        if (is_inverted[pattern])
            bit = 1 - bit;

        byte = (byte << 1) | bit;
        if ((i + 1) % 8 == 0) {
            hidden_data[((i + 1) / 8) - 1] = byte;
            byte = 0;
        }
    }

    // Si no hay encripcion, el siguiente byte deberia ser un .
    if (!is_encryption) {
        char extension[8];
        uint8_t found = 0;
        offset += (*hidden_size) * 8;

        for (uint32_t i = 0; !found; i++) {
            bit = bmp_image->data[offset + i] & 1;

            // printf("Offset is %d\n", offset + i); 
            pattern = (bmp_image->data[offset + i] & PATTERN_MASK) >> 1; // 6 = 110b
            if (is_inverted[pattern])
                bit = 1 - bit;

            byte = (byte << 1) | bit;
            if ((i + 1) % 8 == 0) {
                printf("%c\n", byte);

                extension[((i + 1) / 8) - 1] = byte;
                if (byte == 0) {
                    printf("Found\n");
                    found = 1;
                    hidden_data = realloc(hidden_data, *hidden_size + ((i + 1) / 8));
                    memcpy(hidden_data + *hidden_size, extension, (i + 1) / 8);
                }
                byte = 0;
            }
        }
    }

    return hidden_data;
}

static char * lsb1(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd, uint8_t writes_to_file) {
    if (embed_data_length * 8 > bmp_image->header.image_size_bytes)
        log(FATAL, "Data to embed is too big for the image%s", "");

    char * buff = calloc(bmp_image->header.image_size_bytes, sizeof(char));
    if (buff == NULL)
        log(FATAL, "Could not allocate memory for BMPImage data%s", "");

    memcpy(buff, bmp_image->data, bmp_image->header.image_size_bytes);
    int curr_byte = 0;

    for (uint32_t i = 0; i < embed_data_length; i++) {
        for (int j = 7; j >= 0; j--, curr_byte++) {
            uint8_t bit = (embed_data[i] >> j) & 1;
            buff[curr_byte] = (bmp_image->data[curr_byte] & LSB1_MASK) | (bit);
        }
    }

    if (writes_to_file) {
        write_to_file(out_fd, (char *)&bmp_image->header, HEADER_SIZE);
        write_to_file(out_fd, buff, bmp_image->header.image_size_bytes); // TODO: si es para lsbi no hacer
        close(out_fd);
    }

    return buff;
}


static int lsb4(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd) {
    if (embed_data_length * 2 > bmp_image->header.image_size_bytes)
        log(FATAL, "Data to embed is too big for the image%s", "");

    char * buff = calloc(bmp_image->header.image_size_bytes, sizeof(char));
    if (buff == NULL)
        log(FATAL, "Could not allocate memory for BMPImage data%s", "");

    memcpy(buff, bmp_image->data, bmp_image->header.image_size_bytes);
    int curr_byte = 0;

    for (uint32_t i = 0; i < embed_data_length; i++) {
        for (int j = 1; j >= 0; j--, curr_byte++) {
            uint8_t bits = (embed_data[i] >> (j * 4)) & 0x0F;
            buff[curr_byte] = (bmp_image->data[curr_byte] & LSB4_MASK) | (bits);
        }
    }

    int ret = write_to_file(out_fd, (char *)&bmp_image->header, HEADER_SIZE);
    if (ret == -1)
        log(FATAL, "Could not write to file%s", "");

    ret = write_to_file(out_fd, buff, bmp_image->header.image_size_bytes);
    if (ret == -1)
        log(FATAL, "Could not write to file%s", "");

    close(out_fd);
    free(buff);
    return 0;
}

static int lsbi(BMPImage_ptr bmp_image, char * embed_data, uint32_t embed_data_length, int out_fd) {
    if (embed_data_length * 8 > bmp_image->header.image_size_bytes)
        log(FATAL, "Data to embed is too big for the image%s", "");

    int pattern_counter[4] = { 0 };
    uint8_t is_inverted[4] = { 0 };

    char * steg_image = lsb1(bmp_image, embed_data, embed_data_length, out_fd, 0);
    log(INFO, "Steged image...%s", "");


    uint8_t pattern;
    for (uint32_t i = 0; i < bmp_image->header.image_size_bytes; i++) {
        pattern = (bmp_image->data[i] & PATTERN_MASK) >> 1;
        if ((bmp_image->data[i] & 1) != (steg_image[i] & 1)) {
            pattern_counter[pattern]++;
        }
        else {
            pattern_counter[pattern]--;
        }
    }

    for (uint8_t i = 0; i < 4; i++) {
        if (pattern_counter[i] > 0) {
            log(INFO, "Pattern %d is inverted", i);
            is_inverted[i] = 1;
        }
    }
    log(INFO, "Is inverted: %d, %d, %d, %d\n", is_inverted[0], is_inverted[1], is_inverted[2], is_inverted[3]);

    uint8_t bit;
    for (uint32_t i = 0; i < bmp_image->header.image_size_bytes; i++) {
        pattern = (steg_image[i] & PATTERN_MASK) >> 1;
        if (is_inverted[pattern]) {
            bit = steg_image[i] & 1;
            steg_image[i] = (steg_image[i] & LSB1_MASK) | (1 - bit);
        }
    }

    write_to_file(out_fd, (char *)&bmp_image->header, HEADER_SIZE);
    write_to_file(out_fd, (char *)is_inverted, sizeof(uint32_t));
    write_to_file(out_fd, steg_image, bmp_image->header.image_size_bytes);
    close(out_fd);
    free(steg_image);
    return 0;

}
