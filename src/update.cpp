#include "update.h"

void send_file_to_peer(int source) {
    // primeste de la peer segmentul
    char segment[HASH_SIZE];
    MPI_Recv(segment, HASH_SIZE, MPI_CHAR, source, REQUEST_FOR_CLIENT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // trimite peer-ului ACK-ul
    MPI_Send(NULL, 0, MPI_INT, source, ACK_TAG, MPI_COMM_WORLD);
}

void *upload_thread_func(void *arg) {
    upload_thread_arg upload_arg = *(upload_thread_arg *)arg;

    while(true) {
        // primeste tipul de mesaj
        int type_of_message;
        MPI_Status status;
        MPI_Recv(&type_of_message, 1, MPI_INT, MPI_ANY_SOURCE, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD, &status);

        switch (type_of_message) {
            case REQUEST_FOR_CLIENT_TAG: {
                send_file_to_peer(status.MPI_SOURCE);
                break;
            }

            case STOP_EXECUTION_TAG: {
                printf ("Clientul %d s-a oprit\n", upload_arg.rank);
                return NULL;
            }

            default: {
                printf("Mesaj primit gresit de la clientul %d %d\n", upload_arg.rank, type_of_message);
                break;
            }
        }
    }
}
