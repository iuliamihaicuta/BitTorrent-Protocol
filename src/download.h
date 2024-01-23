#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mpi.h>

#include "define.h"

typedef struct {
    int rank;
    std::vector<std::string> wanted_files;
} download_thread_arg;

void add_segment_to_swarm(std::vector<std::pair<std::string, std::vector<int>>> &swarm_info, std::string segment, int peer);

void recevie_swarm_info_from_tracker(std::vector<std::pair<std::string, std::vector<int>>> &swarm_info, std::string file_name);

void request_swarm_info_from_tracker(std::string file_name, std::vector<std::pair<std::string, std::vector<int>>> &swarm_info);

void send_update_to_tracker(std::string filename, std::vector<std::string> &newly_owned_segments, std::vector<std::pair<std::string, std::vector<int>>> &swarm_info);

void save_file(std::string file_name, int rank, std::vector<std::string> &segments);

void *download_thread_func(void *arg);