# ============================================================================
# Makefile - Monte Carlo Pi Estimation
# ============================================================================
# Mata Kuliah : Komputer Paralel (Sistem Terdistribusi)
# Mahasiswa   : Mhd Aldy Syahputra (NIM: 2322105034)
# Repository  : paralel-2322105034-MhdAldySyahputra
# Opsi        : C - Monte Carlo Simulasi Nilai Pi
# ============================================================================

# Compiler dan flags
CC = gcc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS = -lm
PTHREAD_FLAGS = -pthread
OPENMP_FLAGS = -fopenmp

# Target executables
SERIAL = monte_carlo_serial
PTHREADS = monte_carlo_pthreads
OPENMP = monte_carlo_openmp
BENCH = benchmark

# Jumlah sample default
SAMPLES = 1000000000

# ============================================================================
# Targets
# ============================================================================

.PHONY: all clean run run-serial run-pthreads run-openmp run-benchmark help

## Build semua program
all: $(SERIAL) $(PTHREADS) $(OPENMP) $(BENCH)

## Compile versi serial
$(SERIAL): monte_carlo_serial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

## Compile versi Pthreads
$(PTHREADS): monte_carlo_pthreads.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) -o $@ $< $(LDFLAGS)

## Compile versi OpenMP
$(OPENMP): monte_carlo_openmp.c
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o $@ $< $(LDFLAGS)

## Compile benchmark runner
$(BENCH): benchmark.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) $(OPENMP_FLAGS) -o $@ $< $(LDFLAGS)

## Jalankan semua versi
run: all run-serial run-pthreads run-openmp

## Jalankan versi serial
run-serial: $(SERIAL)
	@echo ""
	@echo "=== Running Serial Version ==="
	./$(SERIAL) $(SAMPLES)

## Jalankan versi Pthreads (default 4 thread)
run-pthreads: $(PTHREADS)
	@echo ""
	@echo "=== Running Pthreads Version ==="
	./$(PTHREADS) $(SAMPLES) 4

## Jalankan versi OpenMP (default 4 thread)
run-openmp: $(OPENMP)
	@echo ""
	@echo "=== Running OpenMP Version ==="
	./$(OPENMP) $(SAMPLES) 4

## Jalankan benchmark lengkap
run-benchmark: $(BENCH)
	@echo ""
	@echo "=== Running Full Benchmark ==="
	./$(BENCH) $(SAMPLES)

## Jalankan benchmark cepat (10^7 samples)
run-quick: $(BENCH)
	@echo ""
	@echo "=== Running Quick Benchmark ==="
	./$(BENCH) 10000000

## Bersihkan file hasil compile
clean:
	rm -f $(SERIAL) $(PTHREADS) $(OPENMP) $(BENCH) *.o *.exe

## Tampilkan bantuan
help:
	@echo "Targets yang tersedia:"
	@echo "  all            - Compile semua program"
	@echo "  run            - Jalankan semua versi"
	@echo "  run-serial     - Jalankan versi serial"
	@echo "  run-pthreads   - Jalankan versi Pthreads"
	@echo "  run-openmp     - Jalankan versi OpenMP"
	@echo "  run-benchmark  - Jalankan benchmark lengkap"
	@echo "  run-quick      - Benchmark cepat (10^7 samples)"
	@echo "  clean          - Hapus file hasil compile"
	@echo ""
	@echo "Variabel:"
	@echo "  SAMPLES=N      - Set jumlah sample (default: $(SAMPLES))"
	@echo ""
	@echo "Contoh:"
	@echo "  make run SAMPLES=100000000"
	@echo "  make run-pthreads SAMPLES=500000000"

