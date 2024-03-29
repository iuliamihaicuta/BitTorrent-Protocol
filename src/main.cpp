#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "define.h"
#include "tracker.h"
#include "peer.h"

int main(int argc, char *argv[])
{
    int numtasks, rank;

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE)
    {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(-1);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Clientul %d porneste\n", rank);

    if (rank == TRACKER_RANK)
    {
        tracker(numtasks, rank);
    }
    else
    {
        peer(numtasks, rank);
    }

    MPI_Finalize();
}
