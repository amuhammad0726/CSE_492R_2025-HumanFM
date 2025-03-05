#ifndef SERVER_H
#define SERVER_H

// New data structures.
// Struct to hold both arrays to manage active classes and volumes.
struct DataPacket {
  int should_terminate;
  int active_classes[20];
  float sound_levels[20];
};

// Function declarations.
int get_class_volume(int class_index);
int start_server();

#endif // SERVER_H
