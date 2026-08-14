#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#define TEST_FILE "test_files/test_10mb.bin"
#define FILE_SIZE (10ULL * 1024 * 1024)

static int read_file(size_t buffer_size, uint64_t *bytes_read)
{
    int fd = open(TEST_FILE, O_RDONLY);

    if (fd == -1) {
        perror("open");
        return -1;
    }

    unsigned char *buffer = malloc(buffer_size);

    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return -1;
    }

    uint64_t total = 0;
    ssize_t n;

    while ((n = read(fd, buffer, buffer_size)) > 0) {
        total += (uint64_t)n;
    }

    if (n < 0) {
        perror("read");
        free(buffer);
        close(fd);
        return -1;
    }

    free(buffer);
    close(fd);

    *bytes_read = total;

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <buffer_size_bytes>\n", argv[0]);
        return 1;
    }

    char *end;
    unsigned long value = strtoul(argv[1], &end, 10);

    if (*argv[1] == '\0' || *end != '\0' || value == 0) {
        fprintf(stderr, "Invalid buffer size: %s\n", argv[1]);
        return 1;
    }

    size_t buffer_size = (size_t)value;
    uint64_t bytes_read = 0;

    if (read_file(buffer_size, &bytes_read) != 0) {
        return 1;
    }

    if (bytes_read != FILE_SIZE) {
        fprintf(stderr,
                "Expected %llu bytes, but read %llu bytes\n",
                (unsigned long long)FILE_SIZE,
                (unsigned long long)bytes_read);
        return 1;
    }

    printf("Buffer size: %zu bytes\n", buffer_size);
    printf("Bytes read: %llu\n",
           (unsigned long long)bytes_read);

    return 0;
}
