#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_SIZE 10000

typedef struct {
    int* array;
    int left;
    int right;
} thread_args;

typedef struct {
    int* src;
    int* dst;
    int width;
    int start_i;
    int end_i;
} merge_args;

void swap(int* a, int* b);
int partition(int arr[], int low, int high);
void quicksort(int arr[], int low, int high);
void* thread_quicksort(void* args);
void* thread_merge_range(void* args);
void parallel_merge(int array[], int num_chunks, thread_args t_args[]);

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

void quicksort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

void* thread_quicksort(void* args) {
    thread_args* t_args = (thread_args*)args;
    quicksort(t_args->array, t_args->left, t_args->right);
    return NULL;
}

void* thread_merge_range(void* args) {
    merge_args* m_args = (merge_args*)args;
    int width = m_args->width;
    int* src = m_args->src;
    int* dst = m_args->dst;

    for (int i = m_args->start_i; i < m_args->end_i; i += 2 * width) {
        int left = i;
        int mid = i + width - 1;
        if (mid >= ARRAY_SIZE) break;
        int right = (i + 2 * width - 1 < ARRAY_SIZE) ? i + 2 * width - 1 : ARRAY_SIZE - 1;

        int l = left;
        int r = mid + 1;
        int k = left;

        while (l <= mid && r <= right) {
            if (src[l] <= src[r]) {
                dst[k++] = src[l++];
            } else {
                dst[k++] = src[r++];
            }
        }
        while (l <= mid) dst[k++] = src[l++];
        while (r <= right) dst[k++] = src[r++];
    }

    return NULL;
}

void parallel_merge(int array[], int num_chunks, thread_args t_args[]) {
    int available_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int* temp = malloc(ARRAY_SIZE * sizeof(int));
    if (!temp) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    int chunk_size = ARRAY_SIZE / num_chunks;
    int* src = array;
    int* dst = temp;

    for (int width = chunk_size; width < ARRAY_SIZE; width *= 2) {
        int num_merges = 0;
        for (int i = 0; i < ARRAY_SIZE; i += 2 * width) {
            num_merges++;
        }

        int num_threads_merge = (num_merges < available_cores) ? num_merges : available_cores;
        if (num_threads_merge < 1) num_threads_merge = 1;

        pthread_t threads[num_threads_merge];
        merge_args m_args[num_threads_merge];

        int merges_per_thread = num_merges / num_threads_merge;
        int remaining_merges = num_merges % num_threads_merge;

        int current_i = 0;
        int thread_idx;
        for (thread_idx = 0; thread_idx < num_threads_merge; thread_idx++) {
            int merges = merges_per_thread + (thread_idx < remaining_merges ? 1 : 0);
            int start_i = current_i;
            int end_i = current_i + merges * 2 * width;
            if (end_i > ARRAY_SIZE) end_i = ARRAY_SIZE;

            m_args[thread_idx].src = src;
            m_args[thread_idx].dst = dst;
            m_args[thread_idx].width = width;
            m_args[thread_idx].start_i = start_i;
            m_args[thread_idx].end_i = end_i;

            current_i = end_i;
        }

        for (thread_idx = 0; thread_idx < num_threads_merge; thread_idx++) {
            pthread_create(&threads[thread_idx], NULL, thread_merge_range, &m_args[thread_idx]);
        }

        for (thread_idx = 0; thread_idx < num_threads_merge; thread_idx++) {
            pthread_join(threads[thread_idx], NULL);
        }

        // Swap src and dst for the next iteration
        int* tmp = src;
        src = dst;
        dst = tmp;
    }

    // If the final result is in temp, copy back to array
    if (src == temp) {
        memcpy(array, temp, ARRAY_SIZE * sizeof(int));
    }

    free(temp);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <max_threads>\n", argv[0]);
        return 1;
    }

    int max_threads = atoi(argv[1]);
    int available_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int num_threads = (max_threads < available_cores) ? max_threads : available_cores;
    printf("Using %d threads (available cores: %d)\n", num_threads, available_cores);

    int array[ARRAY_SIZE];
    srand(time(NULL));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 10000 + 1;
    }

    printf("Unsorted sample: ");
    for (int i = 0; i < 5; i++) printf("%d ", array[i]);
    printf("... ");
    for (int i = ARRAY_SIZE - 5; i < ARRAY_SIZE; i++) printf("%d ", array[i]);
    printf("\n");

    pthread_t threads[num_threads];
    thread_args t_args[num_threads];
    int chunk_size = ARRAY_SIZE / num_threads;

    for (int i = 0; i < num_threads; i++) {
        t_args[i].array = array;
        t_args[i].left = i * chunk_size;
        t_args[i].right = (i == num_threads - 1) ? ARRAY_SIZE - 1 : (i + 1) * chunk_size - 1;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, thread_quicksort, &t_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    parallel_merge(array, num_threads, t_args);

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Threads: %d | Time: %.4f seconds\n", num_threads, elapsed);

    int sorted = 1;
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        if (array[i] > array[i + 1]) {
            sorted = 0;
            break;
        }
    }

    printf(sorted ? "Sorted successfully.\n" : "Sorting failed.\n");

    printf("Sorted sample: ");
    for (int i = 0; i < 5; i++) printf("%d ", array[i]);
    printf("... ");
    for (int i = ARRAY_SIZE - 5; i < ARRAY_SIZE; i++) printf("%d ", array[i]);
    printf("\n");

    return 0;
}
