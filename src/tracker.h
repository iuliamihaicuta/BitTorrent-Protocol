#include <unordered_map>
#include <string>
#include <vector>
#include <mpi.h>
#include <algorithm>

#include "define.h"

void send_swarm_info_to_client(std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> &files);

void update_tracker(int rank, std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> &files);

void tracker(int numtasks, int rank);