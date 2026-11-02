/*
 * ============================================================================
 * Monte Carlo Estimation of Pi - Benchmark Runner
 * ============================================================================
 * 
 * Mata Kuliah : Komputer Paralel (Sistem Terdistribusi)
 * Mahasiswa   : Mhd Aldy Syahputra
 * NIM         : 2322105034
 * Repository  : paralel-2322105034-MhdAldySyahputra
 * Deskripsi   : Program benchmark yang menjalankan versi serial, Pthreads,
 *               dan OpenMP secara bersamaan dalam satu executable untuk
 *               memudahkan perbandingan performa.
 * 
 * Fitur:
 *   - Menjalankan ketiga versi (serial, pthreads, openmp)
 *   - Variasi jumlah thread: 1, 2, 4, 8
 *   - Menghasilkan tabel perbandingan performa
 *   - Menghitung speedup dan efficiency
 * 
 * Compile: gcc -O2 -fopenmp -pthread -o benchmark benchmark.c -lm
 * Run    : ./benchmark [jumlah_sample]
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>
#include <string.h>

#define DEFAULT_NUM_SAMPLES 100000000L  /* 10^8 untuk benchmark cepat */
#define MAX_THREADS 8

/* Struktur untuk thread data (Pthreads) */
typedef struct {
    int thread_id;
    long num_samples;
    unsigned int seed;
    long local_count;
} ThreadData;

/* Variabel global untuk Pthreads */
long g_inside_circle;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Fungsi timing */
double get_wall_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* ==================== VERSI SERIAL ==================== */
double run_serial(long num_samples, double *pi_out) {
    long inside = 0;
    unsigned int seed = 42;
    double start = get_wall_time();
    
    for (long i = 0; i < num_samples; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;
        if (x * x + y * y <= 1.0) inside++;
    }
    
    double elapsed = get_wall_time() - start;
    *pi_out = 4.0 * (double)inside / (double)num_samples;
    return elapsed;
}

/* ==================== WORKER PTHREADS ==================== */
void *pthreads_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long local_inside = 0;
    
    for (long i = 0; i < data->num_samples; i++) {
        double x = (double)rand_r(&data->seed) / RAND_MAX;
        double y = (double)rand_r(&data->seed) / RAND_MAX;
        if (x * x + y * y <= 1.0) local_inside++;
    }
    
    data->local_count = local_inside;
    
    pthread_mutex_lock(&g_mutex);
    g_inside_circle += local_inside;
    pthread_mutex_unlock(&g_mutex);
    
    return NULL;
}

/* ==================== VERSI PTHREADS ==================== */
double run_pthreads(long num_samples, int num_threads, double *pi_out) {
    g_inside_circle = 0;
    
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *tdata = malloc(num_threads * sizeof(ThreadData));
    
    long per_thread = num_samples / num_threads;
    long remainder = num_samples % num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        tdata[i].thread_id = i;
        tdata[i].num_samples = per_thread + (i < remainder ? 1 : 0);
        tdata[i].seed = 42 + i * 1000;
        tdata[i].local_count = 0;
    }
    
    double start = get_wall_time();
    
    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, pthreads_worker, &tdata[i]);
    
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    
    double elapsed = get_wall_time() - start;
    *pi_out = 4.0 * (double)g_inside_circle / (double)num_samples;
    
    free(threads);
    free(tdata);
    return elapsed;
}

/* ==================== VERSI OPENMP ==================== */
double run_openmp(long num_samples, int num_threads, double *pi_out) {
    long inside = 0;
    omp_set_num_threads(num_threads);
    
    double start = get_wall_time();
    
    #pragma omp parallel reduction(+:inside)
    {
        unsigned int seed = 42 + omp_get_thread_num() * 1000;
        long local_n = num_samples / num_threads;
        int tid = omp_get_thread_num();
        
        /* Thread terakhir ambil sisa */
        if (tid == num_threads - 1)
            local_n += num_samples % num_threads;
        
        for (long i = 0; i < local_n; i++) {
            double x = (double)rand_r(&seed) / RAND_MAX;
            double y = (double)rand_r(&seed) / RAND_MAX;
            if (x * x + y * y <= 1.0) inside++;
        }
    }
    
    double elapsed = get_wall_time() - start;
    *pi_out = 4.0 * (double)inside / (double)num_samples;
    return elapsed;
}

/* ==================== MAIN ==================== */
int main(int argc, char *argv[]) {
    long num_samples = DEFAULT_NUM_SAMPLES;
    int thread_counts[] = {1, 2, 4, 8};
    int num_configs = 4;
    
    if (argc > 1) {
        num_samples = atol(argv[1]);
    }
    
    printf("================================================================\n");
    printf("   BENCHMARK: Monte Carlo Pi Estimation\n");
    printf("   Samples: %ld\n", num_samples);
    printf("================================================================\n\n");
    
    /* Run serial baseline */
    double pi_serial, time_serial;
    time_serial = run_serial(num_samples, &pi_serial);
    printf("SERIAL BASELINE:\n");
    printf("  Pi = %.15f | Time = %.4f s\n\n", pi_serial, time_serial);
    
    /* Header tabel */
    printf("%-10s | %-7s | %-15s | %-10s | %-8s | %-10s\n",
           "Method", "Threads", "Pi Estimate", "Time (s)", "Speedup", "Efficiency");
    printf("-----------|---------|-----------------|------------|----------|------------\n");
    printf("%-10s | %-7d | %.13f | %10.4f | %8.2f | %9.2f%%\n",
           "Serial", 1, pi_serial, time_serial, 1.0, 100.0);
    
    /* Run Pthreads with different thread counts */
    for (int i = 0; i < num_configs; i++) {
        int nt = thread_counts[i];
        double pi_pt, time_pt;
        time_pt = run_pthreads(num_samples, nt, &pi_pt);
        double speedup = time_serial / time_pt;
        double efficiency = speedup / nt * 100.0;
        printf("%-10s | %-7d | %.13f | %10.4f | %8.2f | %9.2f%%\n",
               "Pthreads", nt, pi_pt, time_pt, speedup, efficiency);
    }
    
    /* Run OpenMP with different thread counts */
    for (int i = 0; i < num_configs; i++) {
        int nt = thread_counts[i];
        double pi_omp, time_omp;
        time_omp = run_openmp(num_samples, nt, &pi_omp);
        double speedup = time_serial / time_omp;
        double efficiency = speedup / nt * 100.0;
        printf("%-10s | %-7d | %.13f | %10.4f | %8.2f | %9.2f%%\n",
               "OpenMP", nt, pi_omp, time_omp, speedup, efficiency);
    }
    
    printf("\n================================================================\n");
    
    return 0;
}

