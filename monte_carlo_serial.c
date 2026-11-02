/*
 * ============================================================================
 * Monte Carlo Estimation of Pi - Serial Version (Baseline)
 * ============================================================================
 * 
 * Mata Kuliah : Komputer Paralel (Sistem Terdistribusi)
 * Mahasiswa   : Mhd Aldy Syahputra
 * NIM         : 2322105034
 * Repository  : paralel-2322105034-MhdAldySyahputra
 * Deskripsi   : Estimasi nilai Pi menggunakan metode Monte Carlo secara 
 *               serial sebagai baseline untuk perbandingan performa.
 * 
 * Algoritma:
 *   1. Generate N titik random (x, y) dalam kuadran [0,1] x [0,1]
 *   2. Hitung jumlah titik yang jatuh di dalam lingkaran satuan (x^2 + y^2 <= 1)
 *   3. Pi ≈ 4 * (titik_dalam_lingkaran / total_titik)
 * 
 * Compile: gcc -O2 -o monte_carlo_serial monte_carlo_serial.c -lm
 * Run    : ./monte_carlo_serial [jumlah_sample]
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Jumlah sample default: 1 miliar (10^9) */
#define DEFAULT_NUM_SAMPLES 1000000000L

/*
 * Fungsi untuk mendapatkan waktu dalam detik dengan presisi tinggi
 * Menggunakan clock_gettime untuk akurasi nanosecond
 */
double get_time_in_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[]) {
    long num_samples = DEFAULT_NUM_SAMPLES;
    long inside_circle = 0;
    double x, y;
    double pi_estimate;
    double start_time, end_time, elapsed;
    unsigned int seed = 42; /* Seed untuk reproducibility */
    
    /* Parsing argumen command line untuk jumlah sample */
    if (argc > 1) {
        num_samples = atol(argv[1]);
        if (num_samples <= 0) {
            fprintf(stderr, "Error: Jumlah sample harus positif!\n");
            return 1;
        }
    }
    
    printf("============================================\n");
    printf("  Monte Carlo Pi Estimation - Serial\n");
    printf("============================================\n");
    printf("Jumlah sample  : %ld\n", num_samples);
    printf("Seed RNG       : %u\n", seed);
    printf("--------------------------------------------\n");
    
    /* Mulai pengukuran waktu */
    start_time = get_time_in_seconds();
    
    /* 
     * Algoritma Monte Carlo Serial:
     * Loop melalui semua sample, generate titik random,
     * dan hitung yang jatuh dalam lingkaran satuan.
     * 
     * Menggunakan rand_r() dengan seed lokal untuk
     * reproducibility dan konsistensi dengan versi paralel.
     */
    for (long i = 0; i < num_samples; i++) {
        /* Generate koordinat random dalam [0, 1] */
        x = (double)rand_r(&seed) / RAND_MAX;
        y = (double)rand_r(&seed) / RAND_MAX;
        
        /* Cek apakah titik berada di dalam lingkaran satuan */
        if (x * x + y * y <= 1.0) {
            inside_circle++;
        }
    }
    
    /* Hitung estimasi Pi */
    pi_estimate = 4.0 * (double)inside_circle / (double)num_samples;
    
    /* Akhiri pengukuran waktu */
    end_time = get_time_in_seconds();
    elapsed = end_time - start_time;
    
    /* Tampilkan hasil */
    printf("Titik dalam lingkaran : %ld\n", inside_circle);
    printf("Estimasi Pi           : %.15f\n", pi_estimate);
    printf("Nilai Pi sebenarnya   : %.15f\n", M_PI);
    printf("Error absolut         : %.15f\n", fabs(pi_estimate - M_PI));
    printf("Error relatif         : %.10f%%\n", fabs(pi_estimate - M_PI) / M_PI * 100.0);
    printf("Waktu eksekusi        : %.6f detik\n", elapsed);
    printf("============================================\n");
    
    /* Output format CSV untuk parsing otomatis */
    printf("\n[CSV] serial,1,%ld,%.15f,%.6f\n", num_samples, pi_estimate, elapsed);
    
    return 0;
}

