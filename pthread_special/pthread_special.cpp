#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SIZE 100

int n, num_threads;
double A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE], x[MAX_SIZE];

pthread_barrier_t barrier;

void* eliminate(void* arg) {
    int k = *(int*)arg;
    int start = (k + 1) * n / num_threads;
    int end = (k + 2) * n / num_threads;
    for (int i = start; i < end; i++) {
        for (int j = k + 1; j < n; j++) {
            double factor = A[i][k] / A[k][k];
            A[i][j] -= factor * A[k][j];
        }
        B[i] -= A[i][k] * B[k] / A[k][k];
        A[i][k] = 0.0;
    }
    pthread_barrier_wait(&barrier);
    return NULL;
}

void* back_substitute(void* arg) {
    int k = *(int*)arg;
    int start = (k + 1) * n / num_threads - 1;
    int end = k * n / num_threads - 1;
    for (int i = start; i > end; i--) {
        double sum = 0.0;
        for (int j = i + 1; j < n; j++) {
            sum += A[i][j] * x[j];
        }
        x[i] = (B[i] - sum) / A[i][i];
    }
    return NULL;
}

void gauss_elimination() {
    pthread_t threads[num_threads];
    for (int k = 0; k < n - 1; k++) {
        int* arg = malloc(sizeof(*arg));
        *arg = k;
        pthread_create(&threads[k % num_threads], NULL, eliminate, arg);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    for (int k = 0; k < num_threads; k++) {
        int* arg = malloc(sizeof(*arg));
        *arg = k;
        pthread_create(&threads[k], NULL, back_substitute, arg);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

void print_solution() {
    printf("The solution is:\n");
    for (int i = 0; i < n; i++) {
        printf("%lf ", x[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <matrix size> <number of threads>\n", argv[0]);
        exit(1);
    }
    n = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    if (n > MAX_SIZE) {
        printf("Matrix size too large, please increase MAX_SIZE\n");
        exit(1);
    }

    printf("Enter the matrix A:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%lf", &A[i][j]);
        }
    }

    printf("Enter the vector B:\n");
    for (int i = 0; i < n; i++) {
        scanf("%lf", &B[i]);
    }

    pthread_barrier_init(&barrier, NULL, num_threads);

    gauss_elimination();

    print_solution();

    pthread_barrier_destroy(&barrier);

    return 0;
}
