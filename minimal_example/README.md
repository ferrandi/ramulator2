# Minimal Ramulator2 Integration Example

This is the simplest possible example showing how to integrate Ramulator2 as a memory system library into a third-party simulator.

## What This Example Shows

- **Basic Setup**: Initialize Ramulator2 components from a YAML config
- **Request Submission**: Send memory read/write requests to Ramulator2
- **Simulation Loop**: Advance the memory system cycle by cycle
- **Completion Handling**: Process completed requests via callbacks
- **Statistics**: Get memory system performance statistics

## Files

- `main.cpp` - Single-file example with ~80 lines of code
- `config.yaml` - Minimal DDR4 memory configuration
- `Makefile` - Simple build system

## Quick Start

```bash
# Build (requires Ramulator2 to be built first)
make

# Run the example
make test

# Or run manually
LD_LIBRARY_PATH=.. ./minimal_ramulator_example config.yaml
```

## Key Code Points

### 1. Initialize Ramulator2
```cpp
YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});
auto frontend = Ramulator::Factory::create_frontend(config);
auto memory_system = Ramulator::Factory::create_memory_system(config);
frontend->connect_memory_system(memory_system);
memory_system->connect_frontend(frontend);
```

### 2. Submit Memory Request
```cpp
auto callback = [](Ramulator::Request& req) {
    std::cout << "Request completed!\n";
};
bool success = frontend->receive_external_requests(0, address, 0, callback);
```

### 3. Advance Simulation
```cpp
while (requests_pending) {
    memory_system->tick();  // Advance one cycle
}
```

## Expected Output

```
=== Minimal Ramulator2 Integration Example ===
Loading config: config.yaml
Ramulator2 initialized successfully
Issued WRITE request to address 0x0
Issued READ request to address 0x1000
Issued READ request to address 0x2000
Issued WRITE request to address 0x3000
Issued READ request to address 0x4000
Running simulation...
Completed READ to address 0x1000
Completed READ to address 0x2000
Completed READ to address 0x4000
Cycle 10000: 3/5 requests completed
...
=== Simulation Results ===
Total cycles: 45623
Completed requests: 5/5
✓ All requests completed successfully!
```

## Integration into Your Simulator

1. Copy the initialization code from `main.cpp`
2. Use `receive_external_requests()` to submit memory requests
3. Call `memory_system->tick()` to advance memory simulation
4. Handle completions in your callback functions

## Prerequisites

- Ramulator2 must be built first:
  ```bash
  cd .. && cmake -B build && cmake --build build
  ```

This example contains everything needed to understand Ramulator2 integration in under 100 lines of code.