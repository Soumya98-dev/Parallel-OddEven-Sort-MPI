#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Comparison function for qsort
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

void CompareExchangeStandard(int *local_array, int nlocal, int partner, int myrank, MPI_Comm comm) {
    int *recv_array = (int *)malloc(nlocal * sizeof(int));
    MPI_Sendrecv(local_array, nlocal, MPI_INT, partner, 0,
                 recv_array, nlocal, MPI_INT, partner, 0,
                 comm, MPI_STATUS_IGNORE);

    int *temp_array = (int *)malloc(2 * nlocal * sizeof(int));
    for (int i = 0; i < nlocal; i++) temp_array[i] = local_array[i];
    for (int i = 0; i < nlocal; i++) temp_array[nlocal + i] = recv_array[i];
    qsort(temp_array, 2 * nlocal, sizeof(int), compare);

    if (myrank < partner) {
        for (int i = 0; i < nlocal; i++) local_array[i] = temp_array[i];
    } else {
        for (int i = 0; i < nlocal; i++) local_array[i] = temp_array[nlocal + i];
    }

    free(recv_array);
    free(temp_array);
}

int main(int argc, char *argv[]) {
    int n = 128;  // Default array size
    if (argc > 1) n = atoi(argv[1]);

    int myrank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (n % nprocs != 0) {
        MPI_Finalize();
        return -1;
    }

    int nlocal = n / nprocs;
    int *local_array = (int *)malloc(nlocal * sizeof(int));

    if (myrank == 0) {
        int *array = (int *)malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            array[i] = rand() % 129;
        }

        MPI_Scatter(array, nlocal, MPI_INT, local_array, nlocal, MPI_INT, 0, MPI_COMM_WORLD);
        free(array);
    } else {
        MPI_Scatter(NULL, nlocal, MPI_INT, local_array, nlocal, MPI_INT, 0, MPI_COMM_WORLD);
    }

    qsort(local_array, nlocal, sizeof(int), compare);

    double start_time = MPI_Wtime();

    for (int phase = 0; phase < nprocs; phase++) {
        if (phase % 2 == 0) {
            // Even phase
            if (myrank % 2 == 0 && myrank < nprocs - 1) {
                CompareExchangeStandard(local_array, nlocal, myrank + 1, myrank, MPI_COMM_WORLD);
            } else if (myrank % 2 == 1) {
                CompareExchangeStandard(local_array, nlocal, myrank - 1, myrank, MPI_COMM_WORLD);
            }
        } else {
            // Odd phase
            if (myrank % 2 == 1 && myrank < nprocs - 1) {
                CompareExchangeStandard(local_array, nlocal, myrank + 1, myrank, MPI_COMM_WORLD);
            } else if (myrank % 2 == 0 && myrank > 0) {
                CompareExchangeStandard(local_array, nlocal, myrank - 1, myrank, MPI_COMM_WORLD);
            }
        }
    }

    double end_time = MPI_Wtime();

    int *sorted_array = NULL;
    if (myrank == 0) {
        sorted_array = (int *)malloc(n * sizeof(int));
    }

    MPI_Gather(local_array, nlocal, MPI_INT, sorted_array, nlocal, MPI_INT, 0, MPI_COMM_WORLD);

    if (myrank == 0) {
        double execution_time = end_time - start_time;

        // Append execution time to timing_output.txt
        FILE *time_file = fopen("timing_output.txt", "a");
        if (time_file) {
            fprintf(time_file, "[Standard Odd-Even Sort] Array size: %d, Processes: %d, Time: %f seconds\n", n, nprocs, execution_time);
            fclose(time_file);
        }

        // Append the sorted array to result.txt
        FILE *result_file = fopen("result.txt", "a");
        if (result_file) {
            fprintf(result_file, "[Standard Odd-Even Sort] Array size: %d\n", n);
            for (int i = 0; i < n; i++) {
                fprintf(result_file, "%d\n", sorted_array[i]);
            }
            fprintf(result_file, "\n");
            fclose(result_file);
        }

        free(sorted_array);
    }

    free(local_array);
    MPI_Finalize();
    return 0;
}
