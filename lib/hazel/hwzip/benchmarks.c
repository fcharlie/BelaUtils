/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define BENCH_TIME_SECONDS 3

#define BENCHMARKS \
BENCH(deflate_hamlet) \
BENCH(huffman_decode) \
BENCH(huffman_encoder_init) \
BENCH(inflate_hamlet) \
BENCH(lz77_compress) \
BENCH(zlib_deflate_hamlet) \
BENCH(zlib_inflate_hamlet)

/* Declarations for the benchmarking functions. */
#define BENCH(name) \
extern void bench_ ## name(void *aux); \
extern void *bench_ ## name ## _setup(void); \
extern void bench_ ## name ## _teardown(void *aux); \
extern size_t bench_ ## name ## _bytes(void);
BENCHMARKS
#undef BENCH

static bool keep_running;

static void alarm_handler(int sig)
{
        assert(sig == SIGALRM);
        (void)sig;

        keep_running = false;

        /* Don't rely on it getting reset automatically. */
        if (signal(SIGALRM, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(1);
        }
}

static void set_alarm(unsigned secs)
{
        unsigned ret;

        if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
                perror("signal");
                exit(1);
        }

        ret = alarm(secs);
        assert(ret == 0);
}

static double get_time()
{
        struct timeval tv;

        if (gettimeofday(&tv, NULL) != 0) {
                perror("gettimeofday");
                exit(1);
        }

        return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

static void run_benchmark(const char *name, void (*func)(void *aux), void *aux,
                          size_t bytes)
{
        double iterations;
        double start, stop;
        double bytes_per_sec;
        const char *unit;

        keep_running = true;
        iterations = 0;

        printf("%-24s", name);
        fflush(stdout);

        set_alarm(BENCH_TIME_SECONDS);
        start = get_time();

        while (keep_running) {
                func(aux);
                ++iterations;
        }

        stop = get_time();

        printf("%12.0f runs/s", iterations / (stop - start));

        if (bytes != 0) {
                bytes_per_sec = (double)bytes * iterations / (stop - start);

                unit = "B";
                if (bytes_per_sec > 1024) {
                        bytes_per_sec /= 1024;
                        unit = "KB";
                }
                if (bytes_per_sec > 1024) {
                        bytes_per_sec /= 1024;
                        unit = "MB";
                }

                printf("  %6.2f %s/s", bytes_per_sec, unit);
        }

        printf("\n");
}

int main()
{
        void *aux;

#define BENCH(name) \
        aux = bench_ ## name ## _setup(); \
        run_benchmark(#name, bench_ ## name, aux, bench_ ## name ## _bytes()); \
        bench_ ## name ## _teardown(aux);
BENCHMARKS
#undef BENCH

        return 0;
}
