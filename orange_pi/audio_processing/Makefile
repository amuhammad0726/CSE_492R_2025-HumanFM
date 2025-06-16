# Compiler
CXX = g++

# GStreamer flags via pkg-config
GST_CFLAGS := $(shell pkg-config --cflags gstreamer-1.0 gstreamer-app-1.0)
GST_LIBS   := $(shell pkg-config --libs gstreamer-1.0 gstreamer-app-1.0)

# Compilation flags (Add -g for debugging)
CXXFLAGS = -std=c++17 -g -Wall $(GST_CFLAGS)

# Target executable
TARGET = audio_server

# Source files
SRCS = chunks.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build the program.
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(GST_LIBS)

# Compile .cpp files into .o files.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files.
clean:
	rm -f $(OBJS) $(TARGET)