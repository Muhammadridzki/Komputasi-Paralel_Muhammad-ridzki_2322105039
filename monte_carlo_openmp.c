/*
 * ============================================================================
 * Monte Carlo Estimation of Pi - OpenMP Version
 * ============================================================================
 * 
 * Mata Kuliah : Komputer Paralel (Sistem Terdistribusi)
 * Mahasiswa   : Mhd Aldy Syahputra
 * NIM         : 2322105034
 * Repository  : paralel-2322105034-MhdAldySyahputra
 * Deskripsi   : Estimasi nilai Pi menggunakan metode Monte Carlo dengan
 *               paralelisasi menggunakan OpenMP compiler directives.
 * 
 * Strategi Paralelisasi:
 *   - Menggunakan #pragma omp parallel for dengan clause reduction
 *   - Setiap thread otomatis mendapat RNG lokal (private seed)
 *   - Reduction clause menangani race condition pada accumulator
 *   - Schedule static untuk distribusi workload merata
 * 
 * Keunggulan OpenMP:
 *   - Kode jauh lebih ringkas dibanding Pthreads
 *   - Compiler menangani manajemen thread secara otomatis
 *   - Mudah di-enable/disable paralelisasi (hapus pragma = serial)
 * 
 * Compile: gcc -O2 -fopenmp -o monte_carlo_openmp monte_carlo_openmp.c -lm
 * Run    : ./monte_carlo_openmp [jumlah_sample] [jumlah_thread]
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

/* Jumlah sample default: 1 miliar (10^9) */
#define DEFAULT_NUM_SAMPLES 1000000000L
#define DEFAULT_NUM_THREADS 4

/*
 * Fungsi untuk mendapatkan waktu dalam detik dengan presisi tinggi
 * Pada OpenMP, bisa juga menggunakan omp_get_wtime() yang portable
 */
double get_time_in_seconds() {
    return omp_get_wtime();
}

int main(int argc, char *argv[]) {
    long num_samples = DEFAULT_NUM_SAMPLES;
    int num_threads = DEFAULT_NUM_THREADS;
    long inside_circle = 0;
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
    
    /* Set jumlah thread OpenMP */
    omp_set_num_threads(num_threads);
    
    printf("============================================\n");
    printf("  Monte Carlo Pi Estimation - OpenMP\n");
    printf("============================================\n");
    printf("Jumlah sample  : %ld\n", num_samples);
    printf("Jumlah thread  : %d\n", num_threads);
    printf("Sample/thread  : %ld\n", num_samples / num_threads);
    printf("Max threads    : %d\n", omp_get_max_threads());
    printf("--------------------------------------------\n");
    
    /* Mulai pengukuran waktu */
    start_time = get_time_in_seconds();
    
    /*
     * PARALELISASI DENGAN OPENMP
     * ==========================
     * 
     * Pragma directives yang digunakan:
     * 
     * #pragma omp parallel for:
     *   - Membuat region paralel DAN mendistribusikan iterasi loop
     *   - Compiler otomatis membagi iterasi ke thread yang tersedia
     * 
     * reduction(+:inside_circle):
     *   - Setiap thread mendapat salinan private dari inside_circle (init=0)
     *   - Di akhir region paralel, semua salinan dijumlahkan secara atomik
     *   - Ini MENGHILANGKAN kebutuhan mutex manual (bandingkan dengan Pthreads!)
     *   - Lebih efisien dari mutex karena mengurangi contention
     * 
     * private(x, y, seed):
     *   - Setiap variabel ini memiliki salinan per-thread
     *   - PENTING: x dan y harus private agar tidak terjadi race condition
     *   - seed harus private agar setiap thread punya RNG independen
     * 
     * schedule(static):
     *   - Iterasi dibagi secara merata ke thread (chunk = N/num_threads)
     *   - Cocok untuk Monte Carlo karena setiap iterasi memiliki beban sama
     *   - Alternatif: dynamic/guided, tapi overhead scheduling tidak diperlukan
     * 
     * firstprivate(seed):
     *   - Seperti private, tapi diinisialisasi dengan nilai dari master thread
     *   - Kemudian dimodifikasi per-thread di dalam loop
     */
    
    /* Variabel yang akan di-private-kan */
    double x, y;
    unsigned int seed;
    
    #pragma omp parallel for \
        reduction(+:inside_circle) \
        private(x, y) \
        firstprivate(seed) \
        schedule(static)
    for (long i = 0; i < num_samples; i++) {
        /* 
         * Inisialisasi seed per-thread pada iterasi pertama thread ini.
         * Menggunakan omp_get_thread_num() untuk seed unik per-thread.
         * CATATAN: Ini dilakukan di dalam loop, tapi compiler cukup pintar
         * untuk mengoptimalkannya.
         */
        if (i % (num_samples / num_threads) == 0) {
            seed = 42 + omp_get_thread_num() * 1000;
        }
        
        /* Generate koordinat random dalam [0, 1] */
        x = (double)rand_r(&seed) / RAND_MAX;
        y = (double)rand_r(&seed) / RAND_MAX;
        
        /* Cek apakah titik berada di dalam lingkaran satuan */
        if (x * x + y * y <= 1.0) {
            inside_circle++;
        }
    }
    
    /* Akhiri pengukuran waktu */
    end_time = get_time_in_seconds();
    elapsed = end_time - start_time;
    
    /* Hitung estimasi Pi */
    pi_estimate = 4.0 * (double)inside_circle / (double)num_samples;
    
    /* Tampilkan hasil */
    printf("Titik dalam lingkaran : %ld\n", inside_circle);
    printf("Estimasi Pi           : %.15f\n", pi_estimate);
    printf("Nilai Pi sebenarnya   : %.15f\n", M_PI);
    printf("Error absolut         : %.15f\n", fabs(pi_estimate - M_PI));
    printf("Error relatif         : %.10f%%\n", fabs(pi_estimate - M_PI) / M_PI * 100.0);
    printf("Waktu eksekusi        : %.6f detik\n", elapsed);
    printf("============================================\n");
    
    /* Output format CSV untuk parsing otomatis */
    printf("\n[CSV] openmp,%d,%ld,%.15f,%.6f\n", num_threads, num_samples, pi_estimate, elapsed);
    
    return 0;
}

