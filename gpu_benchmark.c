#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cuda_runtime.h>

// Global variables from your code:
long loop_count;
int smCount, cudaCores;
int totalThreads;
double f_avg = 0.0; 
struct timeval t;

// A simple kernel from your code; shortened
__global__ void FAdd(float *d_a, float *d_b, float *d_c, int totalThreads, long l_c)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < totalThreads)
    {
        for (long j = 0; j < l_c; j++)
        {
            // 10 ops
            d_c[i] = d_c[i] + d_c[i];
            d_b[i] = d_b[i] + d_b[i];
            d_a[i] = d_a[i] + d_a[i];
            // ... etc ...
        }
    }
}

// This runs FLOPS once and returns the elapsed time in seconds
double FLOPSOnce()
{
    float *fa, *fb, *fc;
    float *d_fa, *d_fb, *d_fc;
    double start, end;

    fa = (float *)malloc(totalThreads * sizeof(float));
    fb = (float *)malloc(totalThreads * sizeof(float));
    fc = (float *)malloc(totalThreads * sizeof(float));

    cudaMalloc(&d_fa, totalThreads * sizeof(float));
    cudaMalloc(&d_fb, totalThreads * sizeof(float));
    cudaMalloc(&d_fc, totalThreads * sizeof(float));

    for (int i = 0; i < totalThreads; i++) {
        fa[i] = 0.000001f;
        fb[i] = 0.000001f;
        fc[i] = 0.000001f;
    }

    cudaMemcpy(d_fa, fa, totalThreads*sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_fb, fb, totalThreads*sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_fc, fc, totalThreads*sizeof(float), cudaMemcpyHostToDevice);

    // Time start
    gettimeofday(&t, NULL);
    start = t.tv_sec + (t.tv_usec / 1e6);

    // Launch kernel
    FAdd<<< smCount, cudaCores >>>(d_fa, d_fb, d_fc, totalThreads, loop_count);
    cudaDeviceSynchronize();

    // Time end
    gettimeofday(&t, NULL);
    end = t.tv_sec + (t.tv_usec / 1e6);

    // Cleanup
    free(fa);
    free(fb);
    free(fc);
    cudaFree(d_fa);
    cudaFree(d_fb);
    cudaFree(d_fc);

    // Return elapsed time in seconds
    return (end - start);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s [loop_count]\n", argv[0]);
        return 1;
    }
    loop_count = atol(argv[1]);

    // Suppose we already did device setup, found smCount, cudaCores, etc.
    // For a real run, do your usual cudaGetDeviceProperties checks:
    //   smCount = devProp.multiProcessorCount;
    //   cudaCores = ...; 
    // Here, just hardcode something for the example:
    smCount = 1;      // e.g. 1 SM
    cudaCores = 256;  // e.g. 256 threads per SM
    totalThreads = smCount * cudaCores;

    printf("loop_count = %ld\n", loop_count);
    printf("smCount = %d, cudaCores = %d => totalThreads = %d\n",
           smCount, cudaCores, totalThreads);

    // Run FLOPS 10 times & record each time
    int NUM_RUNS = 10;
    double times[NUM_RUNS];
    double sum = 0.0;

    for (int i = 0; i < NUM_RUNS; i++)
    {
        double elapsed = FLOPSOnce();
        times[i] = elapsed;
        sum += elapsed;
    }

    // Print each run’s time
    printf("\nFLOPS Benchmark Times (sec), %d runs:\n", NUM_RUNS);
    for (int i = 0; i < NUM_RUNS; i++)
    {
        printf("  Run %d: %f sec\n", i+1, times[i]);
    }

    // Average time
    double avg_time = sum / NUM_RUNS;
    printf("Avg time: %f sec\n", avg_time);

    // If you want an approximate GFLOPs figure:
    //   total_ops = loop_count * 10 (ops per loop) * totalThreads
    //   then / time => ops/sec => / 1e9 => Gflops
    double total_ops = (double) loop_count * 10.0 * (double) totalThreads;
    double gflops = (total_ops / avg_time) / 1e9;
    printf("Approx. throughput: %.3f GFLOPs\n", gflops);

    return 0;
}

