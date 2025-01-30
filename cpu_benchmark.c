#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>  // needed for sleep()

long loop_count, l_c, th_count;

// We’ll reuse this global struct for timing in the entire program
struct timeval t;

/* -------------------------
   FLOPS Structures & Code
   ------------------------- */

// Thread structure for FLOPS
struct fth
{
    int    lc;
    int    th_counter;
    float  fa, fb, fc, fd;
    pthread_t threads;
};

// Worker function for FLOPS
void *FAdd(void *data)
{
    struct fth *th_d = (struct fth *) data;

    for (th_d->lc = 1; th_d->lc <= l_c; th_d->lc++)
    {
        // 30 floating‐point ops per loop, though never used/stored
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
        th_d->fa + th_d->fd;
        th_d->fa + th_d->fb;
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
        th_d->fa + th_d->fd;
        th_d->fa + th_d->fb;
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;

        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
        th_d->fa + th_d->fd;
        th_d->fa + th_d->fb;
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
        th_d->fa + th_d->fd;
        th_d->fa + th_d->fb;
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;

        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
        th_d->fa + th_d->fd;
        th_d->fa + th_d->fb;
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
        th_d->fa + th_d->fd;
        th_d->fa + th_d->fb;
        th_d->fb + th_d->fc;
        th_d->fa - th_d->fb;
    }
    return NULL;
}

/**
 * Runs the FLOPS benchmark **once**, across `th_count` threads.
 * Returns how long it took in seconds.
 */
double FLOPSBenchmark()
{
    double start_time, end_time, elapsed;
    struct fth ft[th_count];

    // Initialize data for each thread
    for (long i = 0; i < th_count; i++)
    {
        ft[i].lc          = 1;
        ft[i].th_counter  = 1;
        ft[i].fa          = 0.02f;
        ft[i].fb          = 0.2f;
        ft[i].fc          = 0;
        ft[i].fd          = 0;
    }

    // Start timing
    gettimeofday(&t, NULL);
    start_time = t.tv_sec + (t.tv_usec / 1000000.0);

    // Create threads
    for (long c = 0; c < th_count; c++)
    {
        pthread_create(&ft[c].threads, NULL, FAdd, (void *)&ft[c]);
    }
    // Join threads
    for (long m = 0; m < th_count; m++)
    {
        pthread_join(ft[m].threads, NULL);
    }

    // End timing
    gettimeofday(&t, NULL);
    end_time = t.tv_sec + (t.tv_usec / 1000000.0);

    elapsed = end_time - start_time;
    return elapsed;
}

/* -------------------------
   IOPS Structures & Code
   ------------------------- */

// Thread structure for IOPs
struct ith
{
    int    lc;
    int    th_counter;
    int    ia, ib, ic, id;
    pthread_t threads;
};

// Worker function for IOPs
void *IAdd(void *data)
{
    struct ith *th_d = (struct ith *) data;

    for (th_d->lc = 1; th_d->lc <= l_c; th_d->lc++)
    {
        // 30 integer ops per loop
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
        th_d->ia + th_d->id;
        th_d->ia + th_d->ib;
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
        th_d->ia + th_d->id;
        th_d->ia + th_d->ib;
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;

        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
        th_d->ia + th_d->id;
        th_d->ia + th_d->ib;
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
        th_d->ia + th_d->id;
        th_d->ia + th_d->ib;
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;

        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
        th_d->ia + th_d->id;
        th_d->ia + th_d->ib;
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
        th_d->ia + th_d->id;
        th_d->ia + th_d->ib;
        th_d->ib + th_d->ic;
        th_d->ia - th_d->ib;
    }
    return NULL;
}

/**
 * Runs the IOPS benchmark **once**, across `th_count` threads.
 * Returns how long it took in seconds.
 */
double IOPSBenchmark()
{
    double start_time, end_time, elapsed;
    struct ith it[th_count];

    // Initialize data for each thread
    for (long i = 0; i < th_count; i++)
    {
        it[i].lc         = 1;
        it[i].th_counter = 1;
        it[i].ia         = 1;
        it[i].ib         = 2;
        it[i].ic         = 0;
        it[i].id         = 0;
    }

    // Start timing
    gettimeofday(&t, NULL);
    start_time = t.tv_sec + (t.tv_usec / 1000000.0);

    // Create threads
    for (long v = 0; v < th_count; v++)
    {
        pthread_create(&it[v].threads, NULL, IAdd, (void *)&it[v]);
    }
    // Join threads
    for (long n = 0; n < th_count; n++)
    {
        pthread_join(it[n].threads, NULL);
    }

    // End timing
    gettimeofday(&t, NULL);
    end_time = t.tv_sec + (t.tv_usec / 1000000.0);

    elapsed = end_time - start_time;
    return elapsed;
}

/* -------------------------
            main()
   ------------------------- */

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Incorrect number of parameters.\n");
        printf("Usage: %s [operation count] [thread count]\n", argv[0]);
        return 1;
    }

    // Parse arguments
    loop_count = atol(argv[1]); // total loop count
    th_count   = atol(argv[2]); // thread count
    if (th_count <= 0) {
        printf("Invalid thread count.\n");
        return 1;
    }

    // Each thread will do l_c loops
    l_c = loop_count / th_count;

    printf("\nStarting CPU Benchmark...\n");
    printf("  Operation Count: %ld\n", loop_count);
    printf("  Threads Implemented: %ld\n\n", th_count);

    // --------------------------------------------------
    // Run FLOPS Benchmark 10 times
    // --------------------------------------------------
    int NUM_RUNS = 10;
    double flops_times[NUM_RUNS];
    double flops_sum = 0.0;

    for (int i = 0; i < NUM_RUNS; i++)
    {
        double elapsed = FLOPSBenchmark();
        flops_times[i] = elapsed;
        flops_sum += elapsed;
    }

    printf("CPU Benchmark timings (seconds), %d runs:\n", NUM_RUNS);
    for (int i = 0; i < NUM_RUNS; i++)
    {
        printf("  Run %d: %f sec\n", i + 1, flops_times[i]);
    }
    double flops_avg_time = flops_sum / NUM_RUNS;
    printf("Average FLOPS time: %f sec\n", flops_avg_time);

    // Example: If you want to compute a G-FLOPs rate from that average time:
    //    total ops = loop_count * 30 (since each loop does 30 floating ops)
    //    time = flops_avg_time
    //    G-FLOPs = (total ops / time) / 1e9
    double total_flops_ops = (double)loop_count * 30.0;
    double gflops = (total_flops_ops / flops_avg_time) / 1e9;
    printf("Approx. FLOPS throughput: %.3f G-FLOPs\n\n", gflops);

    // Short sleep to separate results visually
    sleep(1);

    // --------------------------------------------------
    // Run IOPS Benchmark 10 times
    // --------------------------------------------------
    double iops_times[NUM_RUNS];
    double iops_sum = 0.0;

    for (int i = 0; i < NUM_RUNS; i++)
    {
        double elapsed = IOPSBenchmark();
        iops_times[i] = elapsed;
        iops_sum += elapsed;
    }

    printf("IOPS timings (seconds), %d runs:\n", NUM_RUNS);
    for (int i = 0; i < NUM_RUNS; i++)
    {
        printf("  Run %d: %f sec\n", i + 1, iops_times[i]);
    }
    double iops_avg_time = iops_sum / NUM_RUNS;
    printf("Average IOPS time: %f sec\n", iops_avg_time);

    double total_iops_ops = (double)loop_count * 30.0;
    double giops = (total_iops_ops / iops_avg_time) / 1e9;
    printf("Approx. IOPS throughput: %.3f G-IOPs\n", giops);

    printf("\nEnding...\n");
    return 0;
}
