#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "server.hpp"

// Core data packet being modified back and forth.
DataPacket data;

// Helper method to extract volume data associated with a specific class.
int get_class_volume(int class_index) {
    return -1;
}

// Helper function to start our server and have listen for incoming data.
int start_server() {
    int server_fd, client_socket;
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

    // Zero out data in our data packet struct.
    memset(&data, 0, sizeof(data));

    // Accepting requests from Node.js until connection is terminated.
    while (true) {
        // Modifying active classes with dummy values.
        for (int i = 0; i < 20; i++) {
            if (std::rand() % 2) {
                data.active_classes[i] ^= 1;
            }
        }

        // Sending struct with initial data to the client Node.js.
        send(client_socket, &data, sizeof(data), 0);

        // Receive data from Node.js client.
        int bytes_received = recv(client_socket, &data, sizeof(data), 0);
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
            std::cout << "Class " << i << ": " << data.active_classes[i]
                        << " | Sound Level: " << data.sound_levels[i] << "\n";
        }

        // Sleep before the next iteration to introduce a slight delay.
        std::cout << "Sleeping...\n";
        sleep(5);
    }

    // Closing client and server sockets
    close(client_socket);
    close(server_fd);

    return 0;
}