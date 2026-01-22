# External Simulator Example with Ramulator2

This example demonstrates how to use Ramulator2 as a memory system library for co-simulation with an external simulator. It provides a complete working example of integrating Ramulator2's cycle-accurate DRAM simulation capabilities into a third-party simulator.

## Overview

The example consists of:
- **MockCPU**: A simple CPU simulator that generates memory requests
- **ExternalSimulator**: Integration layer that connects to Ramulator2
- **External Frontend**: Custom frontend implementation for external simulators
- **Configuration**: YAML-based configuration for memory system parameters

## Key Features

- **Configurable access patterns**: Sequential, random, or mixed memory access
- **Cycle-accurate simulation**: Full integration with Ramulator2's timing models
- **Statistics collection**: Latency, bandwidth, and request type metrics
- **Multiple DRAM models**: Support for DDR4, DDR5, HBM, etc. via configuration
- **Easy integration**: Clean API for external simulators

## Prerequisites

1. **Build Ramulator2 first**:
   ```bash
   cd ..
   cmake -B build
   cmake --build build
   ```

2. **Verify the library exists**:
   ```bash
   ls -la ../libramulator.so
   ```

## Building the External Simulator

### Option 1: Using CMake (Recommended)
```bash
cmake -B build
cmake --build build
```

### Option 2: Using Makefile
```bash
make
make config  # Copy example configuration
```

## Running the Simulator

### Basic Usage
```bash
./external_simulator external_simulator_config.yaml
```

### With Custom Parameters
```bash
./external_simulator external_simulator_config.yaml \
    --pattern random \
    --frequency 3000 \
    --memory-size 2048 \
    --num-requests 50000
```

### Command Line Options
- `--pattern <sequential|random|mixed>`: Memory access pattern
- `--frequency <MHz>`: CPU frequency in MHz
- `--memory-size <MB>`: Memory range to access in MB
- `--num-requests <count>`: Total number of memory requests to issue
- `--help`: Show usage information

## Configuration

The simulator uses YAML configuration files to specify:
- **DRAM type and timing**: DDR4, DDR5, HBM, etc.
- **Memory controller settings**: Scheduling, row policy, refresh
- **Address mapping**: Row-Bank-Rank-Column-Channel organization
- **Clock frequencies**: Memory and controller clocks

### Example Configuration (DDR4-3200)
```yaml
Frontend:
  impl: ExternalSimulator

MemorySystem:
  impl: GenericDRAM
  DRAM:
    impl: DDR4
    org:
      preset: DDR4_3200_8Gx8
    timing:
      preset: DDR4_3200
    Controller:
      Scheduler:
        impl: FRFCFS
      RowPolicy:
        impl: OpenAP

clk: 1666.0  # DDR4-3200 -> 1600 MHz
```

## API Usage for External Simulators

### 1. Initialize Ramulator2
```cpp
// Parse configuration
YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});

// Create components
auto frontend = Ramulator::Factory::create_frontend(config);
auto memory_system = Ramulator::Factory::create_memory_system(config);

// Connect components
frontend->connect_memory_system(memory_system);
memory_system->connect_frontend(frontend);
```

### 2. Send Memory Requests
```cpp
// Create callback for request completion
auto callback = [](Ramulator::Request& req) {
    // Handle completed request
    std::cout << "Request to addr " << req.addr << " completed\n";
};

// Send request (0 = Read, 1 = Write)
bool success = frontend->receive_external_requests(
    0,           // request type (Read)
    address,     // memory address
    0,           // source ID
    callback     // completion callback
);
```

### 3. Advance Simulation
```cpp
// Tick the memory system each cycle
memory_system->tick();
```

### 4. Cleanup
```cpp
frontend->finalize();
memory_system->finalize();
```

## Output and Statistics

The simulator provides detailed statistics including:
- **Simulation cycles**: Total cycles simulated
- **Request counts**: Issued vs completed requests
- **Access breakdown**: Read/write request counts
- **Average latency**: Memory access latency in cycles
- **Bandwidth**: Achieved memory bandwidth in GB/s

Example output:
```
=== Mock CPU Statistics ===
Simulation cycles: 156789
Requests issued: 10000
Requests completed: 10000
Read requests: 7021
Write requests: 2979
Average latency: 45.32 cycles
Bandwidth: 12.34 GB/s
```

## Customization

### Adding New Access Patterns
Extend the `AccessPattern` enum and implement pattern logic in `MockCPU::generate_address()`.

### Custom Frontend Implementation
Create your own frontend by inheriting from `IFrontEnd` and implementing the required interface methods.

### Memory System Configuration
Modify the YAML configuration to use different DRAM types, timing parameters, or controller policies.

## Integration with Your Simulator

To integrate Ramulator2 into your own simulator:

1. **Include the headers**:
   ```cpp
   #include "base/config.h"
   #include "frontend/frontend.h"
   #include "memory_system/memory_system.h"
   ```

2. **Link with libramulator**:
   ```cmake
   target_link_libraries(your_simulator PRIVATE ramulator)
   ```

3. **Follow the API pattern** shown in this example

4. **Configure memory system** via YAML files

## Troubleshooting

- **Library not found**: Ensure Ramulator2 is built first
- **Configuration errors**: Check YAML syntax and available presets
- **Performance issues**: Adjust request generation rate or buffer sizes
- **Build errors**: Verify C++20 compiler support

## References

- [Ramulator2 Paper](https://people.inf.ethz.ch/omutlu/pub/Ramulator2_arxiv23.pdf)
- [Gem5 Integration Example](../resources/gem5_wrappers/)
- [Ramulator2 Documentation](../README.md)