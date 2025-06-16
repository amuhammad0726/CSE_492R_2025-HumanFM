#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include "server.hpp"

// Global variable to provide access to the socket and store current sound values.
int client_socket;
float curr_sounds[20];

// Helper method to extract volume data associated with a specific class.
float get_class_volume(int class_index) {
    // If the index passed was invalid.
    if (class_index < 0 || class_index >= 20) {
        return -1;
    }
    return curr_sounds[class_index];
}

// Helper function used to create a new socket for the C++ server and listen
// for new data from the client.
int server_run() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create our socket for the TCP connection to the Node.js client.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    // Bind the socket to port 4000.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(4000);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed.\n";
        return 1;
    }

    // Listen for any incoming connections.
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Listen failed.\n";
        return 1;
    }

    std::cout << "C++ server listening on port 4000...\n";

    // Accept connection from Node.js client.
    client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_socket < 0) {
        std::cerr << "Connection accept failed.\n";
        return 1;
    }

    std::cout << "Node.js client connected!\n";

    // Listening to data from Node.js until connection is terminated.
    while (true) {
        struct DataPacket data;
        memset(&data, 0, sizeof(data));

        // Receive data from Node.js client.
        ssize_t bytes_received = recv(client_socket, &data, sizeof(data), 0);
        if (bytes_received <= 0) {
            std::cout << "Connection closed by Node.js\n";
            break;
        }

        // Check termination flag to break from loop and close connection.
        if (data.should_terminate == 1) {
            std::cout << "Termination signal received. Closing connection...\n";
            break;
        }

        std::cout << "Updated Data from Node.js:\n";
        for (int i = 0; i < 20; i++) {
            curr_sounds[i] = data.sound_levels[i];
            std::cout << "Class " << i << ": " << data.active_classes[i]
                        << " | Sound Level: " << data.sound_levels[i] << "\n";
        }
    }

    // Closing client and server sockets
    close(client_socket);
    close(server_fd);

    return 0;
}

// Helper function to start our server and have listen for incoming data.
void start_server() {
    // Have the thread call server_run to listen for connection.
    std::thread t1(server_run);
    t1.detach();
    sleep(10);
}

// // Helper function to send data from C++ server to Node client.
int send_server_data(int* class_arr) {
    struct DataPacket data;
    memset(&data, 0, sizeof(data));

    // Loading our array data into the data packet struct.
    for (int i = 0; i < 20; i++) {
        data.active_classes[i] = class_arr[i];
        data.sound_levels[i] = curr_sounds[i];
    }
    std::cout << "Attempting to send client data.\n";
    // Sending struct with initial data to the client Node.js.
    ssize_t bytes_sent = send(client_socket, &data, sizeof(data), 0);

    // Ensuring we actually sent data and the connection wasn't severed.
    if (bytes_sent <= 0) {
        return -1;
    }

    return 0;
}