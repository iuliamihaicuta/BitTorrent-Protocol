#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mpi.h>

#include "define.h"

/*
    files = {
        "file1": [
            "segment1",
            "segment2"
        ],
        "file2": [
            "segment1",
            "segment2",
            "segment3"
        ],
    }
*/

typedef struct {
    int rank;
    std::unordered_map<std::string, std::vector<std::string>> owned_files;
} upload_thread_arg;

void send_file_to_peer(int source);

void *upload_thread_func(void *arg);