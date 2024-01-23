#include "peer.h"

void read_file(int rank, std::vector<std::string> &wanted_files, std::unordered_map<std::string, std::vector<std::string>> &owned_files) {
    std::fstream file;

    char filename[MAX_FILENAME];
    sprintf(filename, "in%d.txt", rank);

    file.open(filename);

    if (!file.is_open()) {
        printf("Eroare la deschiderea fisierului %s\n", filename);
        exit(-1);
    }

    // citesc fisierele detinute si segmentele lor

    int nr_owned_files;
    file >> nr_owned_files;

    for (int i = 0; i < nr_owned_files; i++) {
        char file_name[MAX_FILENAME];
        file >> file_name;

        int nr_segments;
        file >> nr_segments;

        std::vector<std::string> segments;
        for (int j = 0; j < nr_segments; j++) {
            char segment[HASH_SIZE];
            file >> segment;
            segments.push_back(segment);
        }

        owned_files[file_name] = segments;
    }

    // citesc nume fisierele dorite
    int nr_wanted_files;
    file >> nr_wanted_files;

    for (int i = 0; i < nr_wanted_files; i++) {
        char file_name[MAX_FILENAME];
        file >> file_name;
        wanted_files.push_back(file_name);
    }
}

void init_client(int rank, download_thread_arg &download_arg, upload_thread_arg &upload_arg) {
    printf("Clientul %d se initializeaza\n", rank);
    read_file(rank, download_arg.wanted_files, upload_arg.owned_files);

    // trimite tracker-ului numarul fisierelor detinute
    int nr_owned_files = upload_arg.owned_files.size();
    MPI_Send(&nr_owned_files, 1, MPI_INT, TRACKER_RANK, INITIALIZE_TAG, MPI_COMM_WORLD);

    printf("Clientul %d are %d fisiere de trimis\n", rank, nr_owned_files);

    // spune tracker-ului ce fisiere detine
    for (auto it : upload_arg.owned_files) {
        std::string file_name = it.first;
        std::vector<std::string> segments = it.second;

        printf("Clientul %d detine fisierul %s\n", rank, file_name.c_str());

        // trimite tracker-ului numele fisierului si segmentele
        MPI_Send(file_name.c_str(), file_name.size(), MPI_CHAR, TRACKER_RANK, INITIALIZE_TAG, MPI_COMM_WORLD);

        int nr_segments = segments.size();
        MPI_Send(&nr_segments, 1, MPI_INT, TRACKER_RANK, INITIALIZE_TAG, MPI_COMM_WORLD);

        for (auto segment : segments) {
            MPI_Send(segment.c_str(), segment.size(), MPI_CHAR, TRACKER_RANK, INITIALIZE_TAG, MPI_COMM_WORLD);
        }
    }

    printf("Clientul %d a trimis tracker-ului fisierele detinute\n", rank);

    // asteapta de la tracker confirmare
    MPI_Recv(NULL, 0, MPI_INT, TRACKER_RANK, START_EXECUTION_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Clientul %d s-a initializat\n", rank);
}

void peer(int numtasks, int rank)
{
    pthread_t download_thread;
    pthread_t upload_thread;
    void *status;
    int r;

    download_thread_arg download_arg;
    download_arg.rank = rank;

    upload_thread_arg upload_arg;
    upload_arg.rank = rank;

    init_client(rank, download_arg, upload_arg);

    r = pthread_create(&download_thread, NULL, download_thread_func, (void *)&download_arg);
    if (r)
    {
        printf("Eroare la crearea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_create(&upload_thread, NULL, upload_thread_func, (void *)&upload_arg);
    if (r)
    {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_join(download_thread, &status);
    if (r)
    {
        printf("Eroare la asteptarea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_join(upload_thread, &status);
    if (r)
    {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }

    printf("STOP %d\n", rank);
}
