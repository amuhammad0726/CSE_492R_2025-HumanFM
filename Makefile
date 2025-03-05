# Compiler
CXX = g++

# Target executable
TARGET = cpp_server

# Source files
SRCS = main.cpp server.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build the program.
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

# Compile .cpp files into .o files.
%.o: %.cpp
	$(CXX) -c $< -o $@

# Clean build files.
clean:
	rm -f $(OBJS) $(TARGET)