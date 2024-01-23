#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <mpi.h>

#include "define.h"
#include "download.h"
#include "update.h"

void read_file(int rank, std::vector<std::string> &wanted_files, std::unordered_map<std::string, std::vector<std::string>> &owned_files);

void init_client(int rank, download_thread_arg &download_arg, upload_thread_arg &upload_arg);

void peer(int numtasks, int rank);