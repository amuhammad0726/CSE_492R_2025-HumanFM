#include "AudioFile.h"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include <cstring>

/*----- Global Variables ----- */

// Audio file object that we will use to process the WAV file.
AudioFile<float> audioFile;
std::string file_path = "/Users/Aliyan/Desktop/UW Student/CSE Courses/2024-2025/Winter/" +
              std::string("CSE 492R/audio/CSE_492R_Sample1_Steve.wav");
bool isLoaded = audioFile.load(file_path);

/*----- Helper Functions ----- */

// Helper function to provide basic information related to the WAV file.
int print_file_data() {
  // Attempting to load the file data.
  if (!isLoaded) {
    std::cout << "Error loading WAV file!\n";
    return -1;
  } else {
    std::cout << "Success\n";
    audioFile.printSummary();
    return 1;
  }
}

// Function responsible for processing data from audio file in one second chucks.
void print_buffered_samples() {
  int numSamples = audioFile.getNumSamplesPerChannel();
  const int chunkSize = 352; // ~8ms at 44.1kHz
  const int bufferSize = 44100; // 1 second buffer
  static float oneSecondFloatBuf[bufferSize];
  int writeIndex = 0;

  for (int i = 0; i < numSamples; i += chunkSize) {
      int end = std::min(i + chunkSize, numSamples);

      // Process chunk (append to buffer)
      for (int j = i; j < end && writeIndex < bufferSize; ++j) {
        oneSecondFloatBuf[writeIndex++] = audioFile.samples[0][j];
      }

      if (writeIndex >= bufferSize) {
          std::cout << "1-second buffer ready! (" << writeIndex << " samples)\n";
          // send_float_buf(oneSecondFloatBuf, bufferSize);
          writeIndex = 0; // reset for next second
      }
  }
}

// TODO: Send audio data from the buffer to the server.
// Steps:
// 0.) Create a function called wait_for_python_server to have the client connect to the
// server running on the python (may be local).
// 0.1) Have this server function run in main before.
// 1.) Compress the bytes (the audio samples themselves)
// 2.) Sends through the socket (construct a new socket)
// 3.) Ideally find a C++ audio streaming library and then have Malek review. (i.e. G streamer).
// 4.) Python server will return class data back to the client.

/*----- Main Function ----- */

// Puts together the different program pieces above.
int main(int argc, char *argv[]) {
  // Initializing Gstreamer and ensuring that we can process any command line arguments.
  gst_init(&argc, &argv);

  // Creating and parsing a Gstreamer pipeline string.
  // Capturing audio from live microphone, converting to the proper format, preparing
  // and creating packets to be sent to multiple receivers.
  GstElement *pipeline = gst_parse_launch(
      "autoaudiosrc "
      "! audioconvert "
      "! audioresample "
      "! audio/x-raw,rate=16000,format=S16LE,channels=1 "
      "! opusenc "
      "! rtpopuspay "
      "! udpsink "
      "host=239.1.1.1 auto-multicast=true port=5000",
      nullptr
  );

  // Start the pipeline and began recording from microphone.
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  std::cout << "C++ pipeline streaming live audio... Press Ctrl+C to stop.\n";

  // Similar to a while true loop which ensures audio continues to be recorded
  // until we manually sever the connection.
  GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
  g_main_loop_run(loop);

  // Clean-up and freeing of memory.
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  return 0;
}