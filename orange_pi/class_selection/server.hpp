#ifndef SERVER_H
#define SERVER_H

// New data structures.
// Struct to hold both arrays to manage active classes and volumes.
struct DataPacket {
  int should_terminate;
  int active_classes[20];
  float sound_levels[20];
};

// struct ClassPacket {
//   std::byte terminate;
//   std::bitset<20> active_classes;
// };

// struct SoundPacket {
//   std::byte terminate;
//   float sound_levels[20];
// };

// Function declarations.
float get_class_volume(int class_index);
void start_server();
int send_server_data(int* class_arr);

#endif // SERVER_H
