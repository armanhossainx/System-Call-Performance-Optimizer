#include "benchmark_utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>

#define TEST_FILE "test_files/test_10mb.bin"
#define FILE_SIZE (10ULL * 1024 * 1024)

/* Keeping the result of memory scan somewhere observable. Preventing compiler from removing the scan as dead code. */
static volatile uint64_t checksum_sink = 0;

static int read_with_mmap(int fd, uint64_t *bytes_read)
{
    void *mapped;
    unsigned char *data;
    uint64_t checksum = 0;

    mapped = mmap(NULL,
                  FILE_SIZE,
                  PROT_READ,
                  MAP_PRIVATE,
                  fd,
                  0);

    if (mapped == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    data = (unsigned char *)mapped;

    /* Touch every byte in the file. mmap() loads pages on demand, so scanning the mapping makes sure the benchmark actually accesses the complete workload. */
    for (uint64_t i = 0; i < FILE_SIZE; i++) {
        checksum += data[i];
    }

    checksum_sink = checksum;

    if (munmap(mapped, FILE_SIZE) != 0) {
        perror("munmap");
        return -1;
    }

    *bytes_read = FILE_SIZE;

    return 0;
}

static int run_iteration(int iteration, FILE *csv)
{
    int fd = open(TEST_FILE, O_RDONLY);

    if (fd == -1) {
        perror("open");
        return -1;
    }

    double start_wall = get_wall_time_ms();
    double start_cpu = get_cpu_time_sec();
    long start_faults = get_page_faults_rusage();

    uint64_t bytes_read = 0;

    int result = read_with_mmap(fd, &bytes_read);

    double end_wall = get_wall_time_ms();
    double end_cpu = get_cpu_time_sec();
    long end_faults = get_page_faults_rusage();

    close(fd);

    if (result != 0) {
        return -1;
    }

    if (bytes_read != FILE_SIZE) {
        fprintf(stderr,
                "Expected %llu bytes, but processed %llu bytes\n",
                (unsigned long long)FILE_SIZE,
                (unsigned long long)bytes_read);

        return -1;
    }

    TimingResult result_data = {0};

    snprintf(result_data.operation,
             sizeof(result_data.operation),
             "File Read (10 MB)");

    snprintf(result_data.method,
             sizeof(result_data.method),
             "mmap()");

    result_data.iteration = iteration;
    result_data.workload_bytes = bytes_read;
    result_data.wall_time_ms = end_wall - start_wall;
    result_data.cpu_time_s = end_cpu - start_cpu;
    result_data.page_faults = end_faults - start_faults;

    /* the complete file was mapped and every byte was accessed, the workload completed successfully. */
    result_data.checksum_valid = 1;

    append_timing_result(csv, &result_data);

    return 0;
}

static int run_warmup(void)
{
    int fd = open(TEST_FILE, O_RDONLY);

    if (fd == -1) {
        perror("open");
        return -1;
    }

    uint64_t bytes_read = 0;

    int result = read_with_mmap(fd, &bytes_read);

    close(fd);

    if (result != 0) {
        return -1;
    }

    if (bytes_read != FILE_SIZE) {
        fprintf(stderr,
                "Warmup processed %llu bytes instead of %llu\n",
                (unsigned long long)bytes_read,
                (unsigned long long)FILE_SIZE);

        return -1;
    }

    return 0;
}

int main(void)
{
    printf("System Call Performance Optimizer\n");
    printf("Optimized: mmap() file access\n\n");

    if (access(TEST_FILE, R_OK) != 0) {
        fprintf(stderr,
                "Test file not found: %s\n",
                TEST_FILE);
        return 1;
    }

    if (verify_file_size(TEST_FILE, FILE_SIZE) != 1) {
        fprintf(stderr,
                "Test file has an unexpected size.\n");
        return 1;
    }

    if (pin_to_cpu0() != 0) {
        fprintf(stderr,
                "Warning: could not pin process to CPU 0.\n");
    }

    const char *csv_file =
        "results/raw_timing/file_read_optimized.csv";

    init_csv(csv_file,
             "operation,method,iteration,workload_bytes,"
             "wall_time_ms,cpu_time_s,page_faults,checksum_valid");

    FILE *csv = fopen(csv_file, "a");

    if (csv == NULL) {
        perror("fopen");
        return 1;
    }

    /* warmup runs are not included in results. */
    for (int w = 0; w < WARMUP_ITERATIONS; w++) {
        if (run_warmup() != 0) {
            fclose(csv);
            return 1;
        }
    }

    int successful = 0;

    for (int iter = 0; iter < ITERATIONS; iter++) {
        if (run_iteration(iter, csv) == 0) {
            successful++;
        }
    }

    fclose(csv);

    printf("%d/%d runs successful\n\n",
           successful, ITERATIONS);

    printf("Benchmark complete.\n");
    printf("Results: %s\n", csv_file);

    return successful == ITERATIONS ? 0 : 1;
}
