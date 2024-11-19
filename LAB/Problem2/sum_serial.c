#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long long sum_serial(int n) {
    long long sum = 0;
    for (int i = 1; i <= n; i++) {
        sum += i;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./sum_serial <n>\n");
        return 1;
    }

    int n = atoi(argv[1]);

    // Start the timer
    clock_t start = clock();

    long long result = sum_serial(n);

    // Stop the timer
    clock_t end = clock();

    // Calculate the time taken
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Sum from 1 to %d is: %lld\n", n, result);
    printf("Time taken for serial calculation: %f seconds\n", time_taken);

    return 0;
}
