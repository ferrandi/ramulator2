# Minimal Interleaved Ramulator2 Integration Example

CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2

# Paths
RAMULATOR2_ROOT = ..
RAMULATOR2_SRC = $(RAMULATOR2_ROOT)/src
RAMULATOR2_LIB = $(RAMULATOR2_ROOT)/libramulator.so
YAML_CPP_LIB = $(RAMULATOR2_ROOT)/build/_deps/yaml-cpp-build/libyaml-cpp.a

# Include directories
INCLUDES = -I$(RAMULATOR2_SRC) \
           -I$(RAMULATOR2_ROOT)/ext/yaml-cpp/include \
           -I$(RAMULATOR2_ROOT)/ext/spdlog/include

# Libraries
LIBS = -L$(RAMULATOR2_ROOT) -lramulator $(YAML_CPP_LIB) -lpthread

# Target
TARGET = minimal_interleaved_example

# Default target
all: check_libs $(TARGET)

# Check if required libraries exist
check_libs:
	@if [ ! -f $(RAMULATOR2_LIB) ]; then \
		echo "Error: Ramulator2 library not found at $(RAMULATOR2_LIB)"; \
		echo "Please build Ramulator2 first: cd $(RAMULATOR2_ROOT) && cmake -B build && cmake --build build"; \
		exit 1; \
	fi
	@if [ ! -f $(YAML_CPP_LIB) ]; then \
		echo "Error: yaml-cpp library not found at $(YAML_CPP_LIB)"; \
		echo "Please build Ramulator2 first: cd $(RAMULATOR2_ROOT) && cmake -B build && cmake --build build"; \
		exit 1; \
	fi

# Build the target
$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LIBS)
	@echo "Built $(TARGET) successfully"

# Test run
test: $(TARGET)
	@echo "Running interleaved example..."
	LD_LIBRARY_PATH=$(RAMULATOR2_ROOT) ./$(TARGET) config.yaml

# Clean
clean:
	rm -f $(TARGET)

# Help
help:
	@echo "Minimal Interleaved Ramulator2 Integration Example"
	@echo "Usage:"
	@echo "  make          - Build the example"
	@echo "  make test     - Build and run the example"
	@echo "  make clean    - Clean build artifacts"
	@echo "  make help     - Show this help"

.PHONY: all check_libs test clean help