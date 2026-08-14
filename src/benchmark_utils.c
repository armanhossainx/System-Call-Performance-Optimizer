#include "benchmark_utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

/* ============================================================
 * TIMING FUNCTIONS
 * ============================================================ */

double get_wall_time_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime failed");
        return 0.0;
    }
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

double get_cpu_time_sec(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        perror("getrusage failed");
        return 0.0;
    }
    return (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec / 1000000.0 +
           (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec / 1000000.0;
}

long get_page_faults_rusage(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        perror("getrusage failed");
        return 0;
    }
    return usage.ru_minflt + usage.ru_majflt;
}

/* ============================================================
 * CSV LOGGING FUNCTIONS
 * ============================================================ */

void init_csv(const char* filename, const char* headers) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create CSV file: %s\n", filename);
        return;
    }
    fprintf(fp, "%s\n", headers);
    fclose(fp);
}

void append_timing_result(FILE* fp, TimingResult* result) {
    if (!fp || !result) return;
    fprintf(fp, "%s,%s,%d,%lu,%.6f,%.6f,%ld,%d\n",
            result->operation,
            result->method,
            result->iteration,
            (unsigned long)result->workload_bytes,
            result->wall_time_ms,
            result->cpu_time_s,
            result->page_faults,
            result->checksum_valid);
    fflush(fp);
}

void append_syscall_result(FILE* fp, SyscallResult* result) {
    if (!fp || !result) return;
    fprintf(fp, "%s,%s,%d,%ld,%.6f\n",
            result->operation,
            result->method,
            result->iteration,
            result->syscall_count,
            result->syscall_time_ms);
    fflush(fp);
}

void append_perf_result(FILE* fp, PerfResult* result) {
    if (!fp || !result) return;
    fprintf(fp, "%s,%s,%d,%ld,%.6f,%ld\n",
            result->operation,
            result->method,
            result->iteration,
            result->context_switches,
            result->cpu_clock_ms,
            result->page_faults);
    fflush(fp);
}

/* ============================================================
 * FILE UTILITIES
 * ============================================================ */

int generate_test_file(const char* path, uint64_t size_mb) {
    uint64_t size_bytes = size_mb * 1024 * 1024;
    const size_t CHUNK_SIZE = 1024 * 1024; /* 1MB chunks */
    
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        perror("fopen failed");
        return -1;
    }

    unsigned char* chunk = malloc(CHUNK_SIZE);
    if (!chunk) {
        perror("malloc failed");
        fclose(fp);
        return -1;
    }

    /* Fill with a repeating pattern for checksum verification */
    for (size_t i = 0; i < CHUNK_SIZE; i++) {
        chunk[i] = (unsigned char)(i % 256);
    }

    uint64_t written = 0;
    while (written < size_bytes) {
        size_t to_write = (size_bytes - written > CHUNK_SIZE) ? 
                          CHUNK_SIZE : (size_t)(size_bytes - written);
        size_t n = fwrite(chunk, 1, to_write, fp);
        if (n != to_write) {
            perror("fwrite failed");
            free(chunk);
            fclose(fp);
            unlink(path);
            return -1;
        }
        written += n;
    }

    free(chunk);
    fclose(fp);
    
    /* Verify file was created correctly */
    if (verify_file_size(path, size_bytes) != 1) {
        fprintf(stderr, "Error: Test file size verification failed\n");
        unlink(path);
        return -1;
    }
    
    return 0;
}

int verify_file_size(const char* path, uint64_t expected_bytes) {
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("stat failed");
        return -1;
    }
    return (st.st_size == (off_t)expected_bytes) ? 1 : 0;
}

uint64_t compute_checksum(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        perror("fopen failed");
        return 0;
    }

    uint64_t checksum = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        checksum ^= (uint64_t)(unsigned char)c;
        checksum = (checksum << 1) | (checksum >> 63);
    }

    fclose(fp);
    return checksum;
}

int verify_checksum(const char* path, uint64_t expected) {
    uint64_t actual = compute_checksum(path);
    return (actual == expected) ? 1 : 0;
}

/* ============================================================
 * SYSTEM UTILITIES
 * ============================================================ */

int pin_to_cpu0(void) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity failed");
        return -1;
    }
    return 0;
}

int get_cpu_count(void) {
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

/* ============================================================
 * STATISTICS
 * ============================================================ */

static int compare_double(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

Statistics calculate_statistics(double* values, int count) {
    Statistics stats = {0};
    
    if (!values || count <= 0) {
        stats.count = 0;
        return stats;
    }
    
    stats.count = count;

    /* Sort a copy for median and min/max */
    double* sorted = malloc(count * sizeof(double));
    if (!sorted) {
        /* Fallback: compute mean only */
        double sum = 0.0;
        for (int i = 0; i < count; i++) {
            sum += values[i];
        }
        stats.mean = sum / count;
        stats.median = stats.mean;
        stats.stddev = 0.0;
        stats.min = values[0];
        stats.max = values[0];
        return stats;
    }
    
    memcpy(sorted, values, count * sizeof(double));
    qsort(sorted, count, sizeof(double), compare_double);

    stats.min = sorted[0];
    stats.max = sorted[count - 1];

    if (count % 2 == 0) {
        stats.median = (sorted[count/2 - 1] + sorted[count/2]) / 2.0;
    } else {
        stats.median = sorted[count/2];
    }

    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += values[i];
    }
    stats.mean = sum / count;

    double variance = 0.0;
    for (int i = 0; i < count; i++) {
        double diff = values[i] - stats.mean;
        variance += diff * diff;
    }
    stats.stddev = sqrt(variance / count);

    free(sorted);
    return stats;
}
