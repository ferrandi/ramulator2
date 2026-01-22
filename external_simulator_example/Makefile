# Makefile for External Simulator Example
# Requires Ramulator2 to be built first

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

# Paths
RAMULATOR2_ROOT = ..
RAMULATOR2_SRC = $(RAMULATOR2_ROOT)/src
RAMULATOR2_LIB = $(RAMULATOR2_ROOT)/libramulator.so

# Include directories
INCLUDES = -I$(RAMULATOR2_SRC) \
           -I$(RAMULATOR2_ROOT)/ext/yaml-cpp/include \
           -I$(RAMULATOR2_ROOT)/ext/spdlog/include

# Libraries
LIBS = -L$(RAMULATOR2_ROOT) -lramulator -L$(RAMULATOR2_ROOT)/build/_deps/yaml-cpp-build -lyaml-cpp -lpthread

# Source files
SOURCES = main.cpp mock_simulator.cpp external_frontend.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = external_simulator

# Default target
all: check_ramulator $(TARGET)

# Check if Ramulator2 library exists
check_ramulator:
	@if [ ! -f $(RAMULATOR2_LIB) ]; then \
		echo "Error: Ramulator2 library not found at $(RAMULATOR2_LIB)"; \
		echo "Please build Ramulator2 first:"; \
		echo "  cd $(RAMULATOR2_ROOT) && cmake -B build && cmake --build build"; \
		exit 1; \
	fi
	@echo "Found Ramulator2 library: $(RAMULATOR2_LIB)"

# Build the target
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Built $(TARGET) successfully"

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Copy example configuration
config:
	cp $(RAMULATOR2_ROOT)/example_config.yaml .
	@echo "Copied example configuration"

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build artifacts"

# Install (copy to parent directory for easier access)
install: $(TARGET)
	cp $(TARGET) $(RAMULATOR2_ROOT)/
	@echo "Installed $(TARGET) to $(RAMULATOR2_ROOT)/"

# Help
help:
	@echo "External Simulator Makefile"
	@echo "Usage:"
	@echo "  make               - Build the external simulator"
	@echo "  make config        - Copy example configuration file"
	@echo "  make clean         - Clean build artifacts"
	@echo "  make install       - Install to parent directory"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - Ramulator2 must be built first"
	@echo "  - Run: cd .. && cmake -B build && cmake --build build"

.PHONY: all check_ramulator config clean install help