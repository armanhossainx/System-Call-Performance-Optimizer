#ifndef BENCHMARK_UTILS_H
#define BENCHMARK_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <stddef.h>

/* ============================================================
 * BENCHMARK CONFIGURATION
 * ============================================================ */

#define ITERATIONS 20
#define WARMUP_ITERATIONS 3
#define FILE_READ_SIZE_MB 10
#define FILE_COPY_SIZE_MB 100
#define PIPE_DATA_SIZE_MB 10
#define PROCESS_COUNT 1000

/* ============================================================
 * STATISTICS STRUCTURE
 * ============================================================ */

typedef struct {
    int count;
    double mean;
    double median;
    double stddev;
    double min;
    double max;
} Statistics;

/* ============================================================
 * BENCHMARK RESULT STRUCTURES
 * ============================================================ */

typedef struct {
    char operation[128];
    char method[128];
    int iteration;
    uint64_t workload_bytes;
    double wall_time_ms;
    double cpu_time_s;
    long page_faults;
    int checksum_valid;
} TimingResult;

typedef struct {
    char operation[128];
    char method[128];
    int iteration;
    long syscall_count;
    double syscall_time_ms;
} SyscallResult;

typedef struct {
    char operation[128];
    char method[128];
    int iteration;
    long context_switches;
    double cpu_clock_ms;
    long page_faults;
} PerfResult;

/* ============================================================
 * TIMING FUNCTIONS
 * ============================================================ */

double get_wall_time_ms(void);
double get_cpu_time_sec(void);
long get_page_faults_rusage(void);

/* ============================================================
 * CSV LOGGING FUNCTIONS
 * ============================================================ */

void init_csv(const char* filename, const char* headers);

/* ============================================================
 * CSV RESULT FUNCTIONS
 * ============================================================ */

void append_timing_result(FILE* fp, TimingResult* result);
void append_syscall_result(FILE* fp, SyscallResult* result);
void append_perf_result(FILE* fp, PerfResult* result);

/* ============================================================
 * FILE UTILITIES
 * ============================================================ */

int generate_test_file(const char* path, uint64_t size_mb);
int verify_file_size(const char* path, uint64_t expected_bytes);
uint64_t compute_checksum(const char* path);
int verify_checksum(const char* path, uint64_t expected);

/* ============================================================
 * SYSTEM UTILITIES
 * ============================================================ */

int pin_to_cpu0(void);
int get_cpu_count(void);

/* ============================================================
 * STATISTICS
 * ============================================================ */

Statistics calculate_statistics(double* values, int count);

#endif
