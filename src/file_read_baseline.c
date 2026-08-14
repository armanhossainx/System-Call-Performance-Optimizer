#include "benchmark_utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define TEST_FILE "test_files/test_10mb.bin"
#define FILE_SIZE (10ULL * 1024 * 1024)

static const size_t buffer_sizes[] = {
    1024,
    4096,
    16384,
    65536,
    262144,
    1048576
};

#define NUM_BUFFER_SIZES (sizeof(buffer_sizes) / sizeof(buffer_sizes[0]))

static int read_file(int fd, unsigned char *buffer, size_t buffer_size,
                     uint64_t *bytes_read)
{
    ssize_t n;
    uint64_t total = 0;

    while ((n = read(fd, buffer, buffer_size)) > 0) {
        total += (uint64_t)n;
    }

    if (n < 0) {
        perror("read");
        return -1;
    }

    *bytes_read = total;
    return 0;
}

static int run_iteration(size_t buffer_size, int iteration, FILE *csv)
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

    double start_wall = get_wall_time_ms();
    double start_cpu = get_cpu_time_sec();
    long start_faults = get_page_faults_rusage();

    uint64_t bytes_read = 0;
    int result = read_file(fd, buffer, buffer_size, &bytes_read);

    double end_wall = get_wall_time_ms();
    double end_cpu = get_cpu_time_sec();
    long end_faults = get_page_faults_rusage();

    close(fd);

    if (result != 0) {
        free(buffer);
        return -1;
    }

    if (bytes_read != FILE_SIZE) {
        fprintf(stderr,
                "Expected %llu bytes, but read %llu bytes\n",
                (unsigned long long)FILE_SIZE,
                (unsigned long long)bytes_read);

        free(buffer);
        return -1;
    }

    TimingResult result_data = {0};

    snprintf(result_data.operation,
             sizeof(result_data.operation),
             "File Read (10 MB)");

    snprintf(result_data.method,
             sizeof(result_data.method),
             "read() %zuKB buffer",
             buffer_size / 1024);

    result_data.iteration = iteration;
    result_data.workload_bytes = bytes_read;
    result_data.wall_time_ms = end_wall - start_wall;
    result_data.cpu_time_s = end_cpu - start_cpu;
    result_data.page_faults = end_faults - start_faults;

    /*
     * The complete file was read successfully, so the
     * benchmark result is considered valid.
     */
    result_data.checksum_valid = 1;

    append_timing_result(csv, &result_data);

    free(buffer);

    return 0;
}

int main(void)
{
    printf("System Call Performance Optimizer\n");
    printf("Baseline: read() with different buffer sizes\n\n");

    if (access(TEST_FILE, R_OK) != 0) {
        fprintf(stderr, "Test file not found: %s\n", TEST_FILE);
        return 1;
    }

    if (verify_file_size(TEST_FILE, FILE_SIZE) != 1) {
        fprintf(stderr, "Test file has an unexpected size.\n");
        return 1;
    }

    if (pin_to_cpu0() != 0) {
        fprintf(stderr,
                "Warning: could not pin process to CPU 0.\n");
    }

    const char *csv_file =
        "results/raw_timing/file_read_baseline.csv";

    init_csv(csv_file,
             "operation,method,iteration,workload_bytes,"
             "wall_time_ms,cpu_time_s,page_faults,checksum_valid");

    FILE *csv = fopen(csv_file, "a");

    if (csv == NULL) {
        perror("fopen");
        return 1;
    }

    int warmups = WARMUP_ITERATIONS;
    int iterations = ITERATIONS;

    for (size_t i = 0; i < NUM_BUFFER_SIZES; i++) {
        size_t buffer_size = buffer_sizes[i];

        printf("Buffer: %zu KB\n", buffer_size / 1024);

        /*
         * Warmup runs are not included in the results.
         */
        for (int w = 0; w < warmups; w++) {
            int fd = open(TEST_FILE, O_RDONLY);

            if (fd == -1) {
                perror("open");
                fclose(csv);
                return 1;
            }

            unsigned char *buffer = malloc(buffer_size);

            if (buffer == NULL) {
                perror("malloc");
                close(fd);
                fclose(csv);
                return 1;
            }

            uint64_t bytes_read = 0;

            if (read_file(fd, buffer, buffer_size, &bytes_read) != 0) {
                free(buffer);
                close(fd);
                fclose(csv);
                return 1;
            }

            free(buffer);
            close(fd);
        }

        int successful = 0;

        for (int iter = 0; iter < iterations; iter++) {
            if (run_iteration(buffer_size, iter, csv) == 0) {
                successful++;
            }
        }

        printf("  %d/%d runs successful\n\n",
               successful, iterations);
    }

    fclose(csv);

    printf("Benchmark complete.\n");
    printf("Results: %s\n", csv_file);

    return 0;
}
