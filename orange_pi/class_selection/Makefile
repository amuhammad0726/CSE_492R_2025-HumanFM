# Compiler
CXX = g++

# Compilation flags (Add -g for debugging)
CXXFLAGS = -std=c++17 -g -Wall $(GST_CFLAGS)

# Target executable
TARGET = cpp_server

# Source files
SRCS = main.cpp server.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build the program.
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp files into .o files.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files.
clean:
	rm -f $(OBJS) $(TARGET)