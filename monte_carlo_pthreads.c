/*
 * ============================================================================
 * Monte Carlo Estimation of Pi - POSIX Threads (Pthreads) Version
 * ============================================================================
 * 
 * Mata Kuliah : Komputer Paralel (Sistem Terdistribusi)
 * Mahasiswa   : Mhd Aldy Syahputra
 * NIM         : 2322105034
 * Repository  : paralel-2322105034-MhdAldySyahputra
 * Deskripsi   : Estimasi nilai Pi menggunakan metode Monte Carlo dengan
 *               paralelisasi eksplisit menggunakan POSIX Threads.
 * 
 * Strategi Paralelisasi:
 *   - Setiap thread mendapat porsi sample yang sama (N / num_threads)
 *   - Setiap thread memiliki RNG sendiri (seed berbeda) untuk menghindari
 *     race condition dan memastikan kualitas random yang baik
 *   - Setiap thread menghitung counter lokal (inside_circle)
 *   - Hasil akhir digabung menggunakan mutex (reduction manual)
 * 
 * Sinkronisasi:
 *   - Mutex digunakan saat menambahkan hasil lokal ke accumulator global
 *   - Tidak ada shared mutable state selama komputasi utama
 * 
 * Compile: gcc -O2 -pthread -o monte_carlo_pthreads monte_carlo_pthreads.c -lm
 * Run    : ./monte_carlo_pthreads [jumlah_sample] [jumlah_thread]
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

/* Jumlah sample default: 1 miliar (10^9) */
#define DEFAULT_NUM_SAMPLES 1000000000L
#define DEFAULT_NUM_THREADS 4

/* 
 * Struktur data untuk menyimpan argumen dan hasil setiap thread.
 * Setiap thread mendapat copy sendiri agar tidak terjadi race condition.
 */
typedef struct {
    int thread_id;          /* ID thread untuk identifikasi */
    long num_samples;       /* Jumlah sample yang harus diproses thread ini */
    unsigned int seed;      /* Seed RNG per-thread (PENTING: menghindari shared RNG!) */
    long local_count;       /* Hitungan lokal titik dalam lingkaran */
} ThreadData;

/* Variabel global untuk accumulator dan mutex */
long global_inside_circle = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Fungsi untuk mendapatkan waktu dalam detik dengan presisi tinggi
 */
double get_time_in_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/*
 * Fungsi worker yang dijalankan oleh setiap thread.
 * 
 * Setiap thread:
 * 1. Generate titik random menggunakan RNG lokal (rand_r dengan seed per-thread)
 * 2. Hitung berapa titik yang jatuh dalam lingkaran satuan
 * 3. Simpan hasil di local_count
 * 4. Tambahkan hasil ke accumulator global dengan proteksi mutex
 * 
 * CATATAN PENTING: 
 * - Menggunakan rand_r() bukan rand() untuk thread-safety
 * - Setiap thread punya seed berbeda untuk menghindari korelasi
 * - Race condition pada accumulator diatasi dengan mutex
 */
void *monte_carlo_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long local_inside = 0;
    double x, y;
    
    /* Komputasi Monte Carlo - setiap thread proses porsinya sendiri */
    for (long i = 0; i < data->num_samples; i++) {
        /* Generate koordinat random dalam [0, 1] menggunakan RNG per-thread */
        x = (double)rand_r(&data->seed) / RAND_MAX;
        y = (double)rand_r(&data->seed) / RAND_MAX;
        
        /* Cek apakah titik berada di dalam lingkaran satuan */
        if (x * x + y * y <= 1.0) {
            local_inside++;
        }
    }
    
    /* Simpan hasil lokal */
    data->local_count = local_inside;
    
    /* 
     * Critical section: tambahkan hasil lokal ke accumulator global.
     * Mutex diperlukan karena operasi += bukan atomic.
     * Ini adalah REDUCTION manual - analog dengan OpenMP reduction.
     */
    pthread_mutex_lock(&mutex);
    global_inside_circle += local_inside;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    long num_samples = DEFAULT_NUM_SAMPLES;
    int num_threads = DEFAULT_NUM_THREADS;
    double pi_estimate;
    double start_time, end_time, elapsed;
    
    /* Parsing argumen command line */
    if (argc > 1) {
        num_samples = atol(argv[1]);
        if (num_samples <= 0) {
            fprintf(stderr, "Error: Jumlah sample harus positif!\n");
            return 1;
        }
    }
    if (argc > 2) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0 || num_threads > 256) {
            fprintf(stderr, "Error: Jumlah thread harus antara 1-256!\n");
            return 1;
        }
    }
    
    printf("============================================\n");
    printf("  Monte Carlo Pi Estimation - Pthreads\n");
    printf("============================================\n");
    printf("Jumlah sample  : %ld\n", num_samples);
    printf("Jumlah thread  : %d\n", num_threads);
    printf("Sample/thread  : %ld\n", num_samples / num_threads);
    printf("--------------------------------------------\n");
    
    /* Alokasi array thread dan data */
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = malloc(num_threads * sizeof(ThreadData));
    
    if (!threads || !thread_data) {
        fprintf(stderr, "Error: Gagal alokasi memori!\n");
        return 1;
    }
    
    /* 
     * Distribusi workload: bagi sample secara merata ke semua thread.
     * Thread terakhir mendapat sisa pembagian untuk memastikan total tepat.
     */
    long samples_per_thread = num_samples / num_threads;
    long remainder = num_samples % num_threads;
    
    /* Inisialisasi data untuk setiap thread */
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        /* Thread terakhir menangani sisa sample */
        thread_data[i].num_samples = samples_per_thread + (i < remainder ? 1 : 0);
        /* 
         * Seed berbeda per-thread: KRITIS untuk kualitas random!
         * Jika semua thread pakai seed yang sama, mereka generate
         * titik yang identik → hasil tidak akurat.
         */
        thread_data[i].seed = 42 + i * 1000;
        thread_data[i].local_count = 0;
    }
    
    /* Mulai pengukuran waktu */
    start_time = get_time_in_seconds();
    
    /* Buat dan jalankan semua thread */
    for (int i = 0; i < num_threads; i++) {
        int rc = pthread_create(&threads[i], NULL, monte_carlo_worker, &thread_data[i]);
        if (rc) {
            fprintf(stderr, "Error: pthread_create gagal untuk thread %d (rc=%d)\n", i, rc);
            return 1;
        }
    }
    
    /* Tunggu semua thread selesai */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Akhiri pengukuran waktu */
    end_time = get_time_in_seconds();
    elapsed = end_time - start_time;
    
    /* Hitung estimasi Pi */
    pi_estimate = 4.0 * (double)global_inside_circle / (double)num_samples;
    
    /* Tampilkan hasil detail per thread */
    printf("\nDetail per thread:\n");
    for (int i = 0; i < num_threads; i++) {
        printf("  Thread %d: %ld samples, %ld inside circle\n",
               thread_data[i].thread_id,
               thread_data[i].num_samples,
               thread_data[i].local_count);
    }
    
    /* Tampilkan hasil akhir */
    printf("\n--------------------------------------------\n");
    printf("Titik dalam lingkaran : %ld\n", global_inside_circle);
    printf("Estimasi Pi           : %.15f\n", pi_estimate);
    printf("Nilai Pi sebenarnya   : %.15f\n", M_PI);
    printf("Error absolut         : %.15f\n", fabs(pi_estimate - M_PI));
    printf("Error relatif         : %.10f%%\n", fabs(pi_estimate - M_PI) / M_PI * 100.0);
    printf("Waktu eksekusi        : %.6f detik\n", elapsed);
    printf("============================================\n");
    
    /* Output format CSV untuk parsing otomatis */
    printf("\n[CSV] pthreads,%d,%ld,%.15f,%.6f\n", num_threads, num_samples, pi_estimate, elapsed);
    
    /* Cleanup */
    free(threads);
    free(thread_data);
    pthread_mutex_destroy(&mutex);
    
    return 0;
}

