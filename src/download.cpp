#include "download.h"

void add_segment_to_swarm(std::vector<std::pair<std::string, std::vector<int>>> &swarm_info, std::string segment, int peer) {
    bool found = false;
    for (auto &it : swarm_info) {
        if (it.first == segment) {
            // verifica daca peer-ul apare deja ca detinator al segmentului
            if (it.second.end() == std::find(it.second.begin(), it.second.end(), peer)) {
                it.second.push_back(peer);
            }
            found = true;
        }
    }

    if (!found) {
        swarm_info.push_back(std::make_pair(segment, std::vector<int>()));
        swarm_info.back().second.push_back(peer);
    }

}

void recevie_swarm_info_from_tracker(std::vector<std::pair<std::string, std::vector<int>>> &swarm_info, std::string file_name) {
    // trimite tracker-ului numele fisierului
    MPI_Send(file_name.c_str(), MAX_FILENAME, MPI_CHAR, TRACKER_RANK, REQUEST_FOR_TRACKER_TAG, MPI_COMM_WORLD);
    
    // primeste de la tracker numarul de segmente
    int nr_segments;
    MPI_Recv(&nr_segments, 1, MPI_INT, TRACKER_RANK, SEND_SWARM_INFO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Clientul va primit de la tracker %d segmente\n", nr_segments);

    // primeste de la tracker segmentele si cine le detine
    for (int i = 0; i < nr_segments; ++i) {
        // primeste de la tracker segmentul
        char segment[HASH_SIZE];
        MPI_Recv(segment, HASH_SIZE, MPI_CHAR, TRACKER_RANK, SEND_SWARM_INFO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int nr_peers;
        MPI_Recv(&nr_peers, 1, MPI_INT, TRACKER_RANK, SEND_SWARM_INFO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int j = 0; j < nr_peers; ++j) {
            int peer;
            MPI_Recv(&peer, 1, MPI_INT, TRACKER_RANK, SEND_SWARM_INFO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            add_segment_to_swarm(swarm_info, segment, peer);
        }
    }

    printf("Clientul a primit swarm info de la tracker\n");
}

void request_swarm_info_from_tracker(std::string file_name, std::vector<std::pair<std::string, std::vector<int>>> &swarm_info) {
    // trimite tracker-ului tipul de mesaj
    int type_of_message = REQUEST_FOR_TRACKER_TAG;
    MPI_Send(&type_of_message, 1, MPI_INT, TRACKER_RANK, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD);

    recevie_swarm_info_from_tracker(swarm_info, file_name);
}

void send_update_to_tracker(std::string filename, std::vector<std::string> &newly_owned_segments, std::vector<std::pair<std::string, std::vector<int>>> &swarm_info) {
    // trimite tracker-ului tipul de mesaj
    int type_of_message = UPDATE_TRACKER_TAG;
    MPI_Send(&type_of_message, 1, MPI_INT, TRACKER_RANK, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD);

    // trimite tracker-ului numele fisierului
    MPI_Send(filename.c_str(), MAX_FILENAME, MPI_CHAR, TRACKER_RANK, UPDATE_TRACKER_TAG, MPI_COMM_WORLD);

    // trimite tracker-ului numarul de segmente
    int nr_segments = newly_owned_segments.size();
    MPI_Send(&nr_segments, 1, MPI_INT, TRACKER_RANK, UPDATE_TRACKER_TAG, MPI_COMM_WORLD);

    // trimite tracker-ului segmentele si cine le detine
    for (auto it : newly_owned_segments) {
        // trimite tracker-ului segmentul
        MPI_Send(it.c_str(), HASH_SIZE, MPI_CHAR, TRACKER_RANK, UPDATE_TRACKER_TAG, MPI_COMM_WORLD);
    }

    recevie_swarm_info_from_tracker(swarm_info, filename);
}

void save_file(std::string file_name, int rank, std::vector<std::string> &segments) {
    FILE *file;

    char filename[MAX_FILENAME];
    sprintf(filename, "client%d_%s", rank, file_name.c_str());

    file = fopen(filename, "w");

    for (auto it : segments) {
        fprintf(file, "%.32s\n", it.c_str());
    }

    fclose(file);
}

void *download_thread_func(void *arg)
{
    download_thread_arg download_arg = *(download_thread_arg *)arg;

    // nume fisier -> index segment 
    std::unordered_map<std::string, std::vector<std::string>> newly_owned_segments;

    for (auto file_name : download_arg.wanted_files) {

        printf("Clientul %d cere fisierul %s\n", download_arg.rank, file_name.c_str());

        std::vector<std::pair<std::string, std::vector<int>>> swarm_info;
        request_swarm_info_from_tracker(file_name, swarm_info);

        // cere segmente de la peeri
        int index = 0;
        newly_owned_segments[file_name] = std::vector<std::string>();
        for (auto it : swarm_info) {

            if (index % 10 == 0 && index != 0) {
                // updateaza tracker-ul cu segmentele detinute
                send_update_to_tracker(file_name, newly_owned_segments[file_name], swarm_info);
            }

            std::string segment = it.first;
            std::vector<int> peers = it.second;

            // cere segmentul de la un peer random
            int peer = peers[rand() % peers.size()];

            // trimite peer-ului tipul de mesaj
            int type_of_message = REQUEST_FOR_CLIENT_TAG;
            MPI_Send(&type_of_message, 1, MPI_INT, peer, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD);

            // trimite peer-ului segmentul
            MPI_Send(segment.c_str(), HASH_SIZE, MPI_CHAR, peer, REQUEST_FOR_CLIENT_TAG, MPI_COMM_WORLD);
            
            // primeste ACK de la peer
            MPI_Recv(NULL, 0, MPI_INT, peer, ACK_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            newly_owned_segments[file_name].push_back(segment);

            index++;
        }

        // updateaza tracker-ul ca detine segmentele
        send_update_to_tracker(file_name, newly_owned_segments[file_name], swarm_info);
        save_file(file_name, download_arg.rank ,newly_owned_segments[file_name]);

        printf("Clientul %d a terminat de descarcat fisierul %s\n", download_arg.rank, file_name.c_str());
    }

    // trimite tracker-ului tipul de mesaj
    int type_of_message = ENDED_ALL_DOWNLOADS_TAG;
    MPI_Send(&type_of_message, 1, MPI_INT, TRACKER_RANK, TYPE_OF_MESSAGE_TAG, MPI_COMM_WORLD);

    printf("Clientul %d a terminat toate download-urile\n", download_arg.rank);

    return NULL;
}
