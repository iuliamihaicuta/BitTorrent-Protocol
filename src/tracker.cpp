#include "tracker.h"

void add_segment_to_file(std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<int>>>> &files,
                            std::string segment, std::string file_name, int rank) {
    // verifica daca segmentul exista deja
    bool found = false;

    for (auto it : files[file_name]) {
        if (it.first == segment) {
            // verifica daca clientul apare deja ca detinator al segmentului
            if (it.second.end() == std::find(it.second.begin(), it.second.end(), rank)) {
                it.second.push_back(rank);
            }
            found = true;
        }
    }

    if (!found) {
        files[file_name].push_back(std::make_pair(segment, std::vector<int>()));
        files[file_name][files[file_name].size() - 1].second.push_back(rank);
    }
}

void send_swarm_info_to_client(std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<int>>>> &files) {
    char file_name[MAX_FILENAME];
    MPI_Status status;
    MPI_Recv(file_name, MAX_FILENAME, MPI_CHAR, MPI_ANY_SOURCE, REQUEST_FOR_TRACKER_TAG, MPI_COMM_WORLD, &status);

    printf("Tracker-ul a primit cerere de la clientul %d pentru fisierul %s\n", status.MPI_SOURCE, file_name);

    // trimit client + ce segmente are fiecare peer
    if (files.find(file_name) != files.end()) {
        int nr_segments = files[file_name].size();
        MPI_Send(&nr_segments, 1, MPI_INT, status.MPI_SOURCE, SEND_SWARM_INFO, MPI_COMM_WORLD);

        printf("Tracker-ul trimite clientului %d %d segmente\n", status.MPI_SOURCE, nr_segments);

        for (auto it : files[file_name]) {
            std::string segment = it.first;
            std::vector<int> peers = it.second;

            MPI_Send(segment.c_str(), HASH_SIZE, MPI_CHAR, status.MPI_SOURCE, SEND_SWARM_INFO, MPI_COMM_WORLD);

            int nr_peers = peers.size();
            MPI_Send(&nr_peers, 1, MPI_INT, status.MPI_SOURCE, SEND_SWARM_INFO, MPI_COMM_WORLD);

            for (auto peer : peers) {
                MPI_Send(&peer, 1, MPI_INT, status.MPI_SOURCE, SEND_SWARM_INFO, MPI_COMM_WORLD);
            }
        }

        printf("Tracker-ul a trimis clientului %d informatiile despre swarm\n", status.MPI_SOURCE);

    } else {
        exit(-1);
    }

}

void update_tracker(int rank, std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<int>>>> &files) {
    
    printf("Tracker-ul se actualizeaza cu informatiile de la clientul %d\n", rank);
    
    // primeste de la client numele fisierului
    char file_name[MAX_FILENAME];
    MPI_Recv(file_name, MAX_FILENAME, MPI_CHAR, rank, UPDATE_TRACKER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    // primeste de la client numarul de segmente
    int nr_segments;
    MPI_Recv(&nr_segments, 1, MPI_INT, rank, UPDATE_TRACKER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int i = 0; i < nr_segments; ++i) {
        // primeste de la client segmentul
        char segment[HASH_SIZE];
        MPI_Recv(segment, HASH_SIZE, MPI_CHAR, rank, UPDATE_TRACKER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        add_segment_to_file(files, segment, file_name, rank);
    }

    // trimite clientului lista de peeri din swarm
    send_swarm_info_to_client(files);
}

void tracker(int numtasks, int rank)
{
    /*
        files = {
            "file1": [
                ("segment1", [1, 2, 3]),
                ("segment2", [1, 2, 3])
            ],
            "file2": [
                ("segment1", [1, 3]),
                ("segment2", [1, 2, 3]),
                ("segment3", [2, 3])
            ]
        }
    */
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<int>>>> files;
    
    // initializeaza tracker-ul
    for (int i = 1; i < numtasks; ++i) {
        // primeste de la fiecare client numele fisierelor si segmentele detinute
        int nr_owned_files;
        MPI_Recv(&nr_owned_files, 1, MPI_INT, i, INITIALIZE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Tracker-ul a primit de la clientul %d %d fisiere\n", i, nr_owned_files);

        for (int j = 0; j < nr_owned_files; j++) {
            char file_name[MAX_FILENAME];
            MPI_Recv(file_name, MAX_FILENAME, MPI_CHAR, i, INITIALIZE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printf("Tracker-ul a primit de la clientul %d fisierul %s\n", i, file_name);

            int nr_segments;
            MPI_Recv(&nr_segments, 1, MPI_INT, i, INITIALIZE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int k = 0; k < nr_segments; k++) {
                char segment[HASH_SIZE + 1];
                MPI_Recv(segment, HASH_SIZE, MPI_CHAR, i, INITIALIZE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                segment[HASH_SIZE] = '\0';

                add_segment_to_file(files, segment, file_name, i);
            }
        }
    }

    // afiseaza tot files intr un format frumos
    for (auto it : files) {
        printf("%s\n", it.first.c_str());
        for (auto it2 : it.second) {
            printf("\t%s\n", it2.first.c_str());
            for (auto it3 : it2.second) {
                printf("\t\t%d\n", it3);
            }
        }
    }

    // broadcast confirmare
    for (int i = 1; i < numtasks; ++i) {
        MPI_Send(NULL, 0, MPI_INT, i, START_EXECUTION_TAG, MPI_COMM_WORLD);
    }

    printf("Tracker-ul s-a initializat\n");

    int ended_clients = 0;

    // asteapta cereri de la clienti
    while (true) {
        int type_of_message;
        MPI_Status status;
        MPI_Recv(&type_of_message, 1, MPI_INT, MPI_ANY_SOURCE, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD, &status);

        printf("Tracker-ul a primit mesaj de tip %d de la clientul %d\n", type_of_message, status.MPI_SOURCE);

        switch (type_of_message) {
            case REQUEST_FOR_TRACKER_TAG: {
                send_swarm_info_to_client(files);
                break;
            }

            case UPDATE_TRACKER_TAG: {
                update_tracker(status.MPI_SOURCE, files);
                break;
            }

            case ENDED_ALL_DOWNLOADS_TAG: {
                printf("END ALL %d %d\n", ended_clients, status.MPI_SOURCE);
                ended_clients++;
                
                if (ended_clients == numtasks - 1) {
                    printf("Tracker-ul a primit mesaj de la toti clientii ca au terminat download-ul\n");
                    for (int i = 1; i < numtasks; ++i) {
                        int type = STOP_EXECUTION_TAG;
                        MPI_Send(&type, 1, MPI_INT, i, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD);
                    }

                    printf("Tracker-ul a terminat executia\n");
                    return;
                }
                break;
            }
        }
    }
}
