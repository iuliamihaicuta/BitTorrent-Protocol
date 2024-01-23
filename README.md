# BitTorrent Protocol

## 1. Description

This program implements a peer-to-peer (P2P) network for file sharing. Using the Message Passing Interface (MPI) application programming interfaces (APIs), the program enables efficient data transfer between network nodes. The program's structure is divided into several main modules, each with specific responsibilities:

* `peer.cpp` și `peer.h`
    * Handle the basic functionalities of a peer in the network, including reading input files, initializing the client, and coordinating the general activities of the peer.

* `tracker.cpp` și `tracker.h`
    * The tracker is the central coordination point of the network, managing information about available files and the nodes that possess them.
    * Handle tracker initialization and manage requests from peers.

* `download.cpp` și `download.h`
    * Handle the file downloading process, including requesting and receiving information from the tracker and managing downloads from other peers.

* `update.cpp` și `update.h`
    * Responsible for updating and transferring data between peers, these files facilitate the exchange of file segments.

## 2. Initialization

Upon program initialization, each peer reads its configuration from a specified input file. This process includes identifying the files the peer holds and wishes to download.

After reading the data, the peer connects to the network, initializing communication with the tracker and other peers. Initialization includes configuring threads for downloading and uploading, as well as establishing the necessary connections for MPI communication.

## 3. Update
Nodes holding specific segments of a file can connect to other nodes to send those segments. This process is managed by the update.cpp and update.h modules.

Based on the information received from the tracker, the peer initiates connections with other peers and begins data transfer. This process runs in parallel with the download process, allowing the peer to both download and upload simultaneously.

A peer's files are stored in memory in the following format:
```
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
```

## 4. Download
The download process is a key aspect of the P2P network's operation. The download.cpp and download.h modules handle downloading files from other peers.

This process begins by requesting information from the tracker about the locations of desired file segments. Based on this information, the peer initiates connections with peers that hold the necessary segments and starts the download. Mechanisms for managing swarms ensure efficient distribution of download tasks and maintain a balance between supply and demand in the network.

To enable the tracker to handle download requests, it stores information about each segment and which peer possesses it in the following format:
```
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
```

---