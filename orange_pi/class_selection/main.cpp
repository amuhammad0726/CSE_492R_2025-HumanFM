#include <iostream>
#include "server.hpp"
#include <unistd.h>

int main() {
  // We first start the server through the main thread to listen to connections.
  // A child thread will be created which will attempt to listen in a loop
  // for data from the client or Node.
  start_server();

  while (true) {
    // Creating an array of dummy values to send across the socket.
    int class_arr[20];
    memset(&class_arr, 0, sizeof(class_arr));
    for (int i = 0; i < 20; i++) {
      if (std::rand() % 2) {
        class_arr[i] ^= 1;
      }
    }
    int status = send_server_data(class_arr);
    if (status == -1) {
      break;
    }
    sleep(10);
  }

  std::cout << "We have completed testing in the main function!\n";
}