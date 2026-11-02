# Monte Carlo Pi Estimation - Parallel Computing

## Informasi Mahasiswa & Tugas
- **Nama** : Mhd Aldy Syahputra
- **NIM** : 2322105034
- **Program Studi** : Teknik Informatika
- **Mata Kuliah** : Komputer Paralel (Sistem Terdistribusi)
- **Dosen** : Dr. Ir. Antonius Irianto. S, ST., MT
- **Opsi Tugas** : Opsi C - Monte Carlo Simulasi Nilai π (Difficulty: 1.5x)
- **Repository GitHub** : `paralel-2322105034-MhdAldySyahputra`

## Deskripsi
Implementasi estimasi nilai π (Pi) menggunakan metode Monte Carlo dengan tiga pendekatan:
1. **Serial** - Versi baseline tanpa paralelisasi
2. **POSIX Threads (Pthreads)** - Paralelisasi eksplisit/manual
3. **OpenMP** - Paralelisasi dengan compiler directives

Tugas UTS Mata Kuliah Komputer Paralel (Sistem Terdistribusi) - Opsi C (Difficulty 1.5x)

## Metode Monte Carlo
Metode ini mengestimasi nilai Pi dengan:
1. Generate N titik random (x, y) dalam kuadran [0,1] × [0,1]
2. Hitung rasio titik yang jatuh di dalam lingkaran satuan (x² + y² ≤ 1)
3. Pi ≈ 4 × (titik dalam lingkaran / total titik)

## Struktur File
```
.
├── monte_carlo_serial.c      # Versi serial (baseline)
├── monte_carlo_pthreads.c    # Versi Pthreads (paralel eksplisit)
├── monte_carlo_openmp.c      # Versi OpenMP (compiler directive)
├── benchmark.c               # Benchmark runner (semua versi)
├── Makefile                  # Build automation
└── README.md                 # Dokumentasi ini
```

## Persyaratan
- **Compiler**: GCC dengan dukungan Pthreads dan OpenMP
- **OS**: Linux (direkomendasikan) atau Windows dengan MinGW/WSL
- **Library**: libm (math), libpthread

## Cara Compile

### Compile semua program sekaligus:
```bash
make all
```

### Compile individual:
```bash
# Versi serial
gcc -O2 -o monte_carlo_serial monte_carlo_serial.c -lm

# Versi Pthreads
gcc -O2 -pthread -o monte_carlo_pthreads monte_carlo_pthreads.c -lm

# Versi OpenMP
gcc -O2 -fopenmp -o monte_carlo_openmp monte_carlo_openmp.c -lm

# Benchmark runner
gcc -O2 -pthread -fopenmp -o benchmark benchmark.c -lm
```

## Cara Run

### Menggunakan Makefile:
```bash
# Jalankan semua versi (1 miliar sample)
make run

# Benchmark lengkap dengan variasi thread (1, 2, 4, 8)
make run-benchmark

# Benchmark cepat (10 juta sample)
make run-quick

# Custom jumlah sample
make run SAMPLES=500000000
```

### Jalankan manual:
```bash
# Serial: ./monte_carlo_serial [jumlah_sample]
./monte_carlo_serial 1000000000

# Pthreads: ./monte_carlo_pthreads [jumlah_sample] [jumlah_thread]
./monte_carlo_pthreads 1000000000 4

# OpenMP: ./monte_carlo_openmp [jumlah_sample] [jumlah_thread]
./monte_carlo_openmp 1000000000 4

# Benchmark: ./benchmark [jumlah_sample]
./benchmark 100000000
```

## Spesifikasi Sistem Pengujian
- **CPU**: AMD Ryzen 5 7535HS (6 cores, 12 threads)
- **RAM**: 14 GB
- **OS**: Windows 11 / Linux (WSL)

## Catatan Teknis
- RNG menggunakan `rand_r()` dengan seed per-thread untuk menghindari race condition
- Sinkronisasi Pthreads menggunakan mutex pada accumulator global
- OpenMP menggunakan `reduction(+:)` clause untuk atomic summation
- Seed default: 42 (+ offset per thread) untuk reproducibility

