#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int start;
    int end;
    long long result;
} ThreadData;

void* sum_partial(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    long long partial_sum = 0;
    for (int i = data->start; i <= data->end; i++) {
        partial_sum += i;
    }
    data->result = partial_sum;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./sum_multi_thread <numThreads> <n>\n");
        return 1;
    }

    int numThreads = atoi(argv[1]);
    int n = atoi(argv[2]);

    pthread_t threads[numThreads];
    ThreadData thread_data[numThreads];

    // Start the timer
    clock_t start = clock();

    int range = n / numThreads;

    for (int i = 0; i < numThreads; i++) {
        thread_data[i].start = i * range + 1;
        thread_data[i].end = (i == numThreads - 1) ? n : (i + 1) * range;
        pthread_create(&threads[i], NULL, sum_partial, &thread_data[i]);
    }

    long long total_sum = 0;
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
        total_sum += thread_data[i].result;
    }

    // Stop the timer
    clock_t end = clock();

    // Calculate the time taken
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Sum from 1 to %d is: %lld\n", n, total_sum);
    printf("Time taken for multi-thread calculation: %f seconds\n", time_taken);

    return 0;
}
