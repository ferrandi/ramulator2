# Quick Start Guide: External Simulator with Ramulator2

This guide shows how to quickly get started with the external simulator example that demonstrates Ramulator2 integration.

## 🚀 Quick Start (5 minutes)

### Step 1: Build the External Simulator
```bash
# Navigate to the external simulator example directory
cd external_simulator_example

# Build using CMake (recommended)
cmake -B build
cmake --build build

# OR build using Makefile
make
```

### Step 2: Run the Demo
```bash
# Run the comprehensive demo script
./run_example.sh

# OR run manually with specific parameters
./external_simulator external_simulator_config.yaml --pattern sequential --num-requests 20
```

## 📋 What You'll See

The simulator will output:
1. **Initialization**: Ramulator2 components loading
2. **Request Generation**: Memory read/write requests being issued
3. **Cycle Progress**: Simulation advancing cycle by cycle
4. **Statistics**: Performance metrics at completion

Example output:
```
=== External Simulator with Ramulator2 ===
Configuration file: external_simulator_config.yaml
CPU frequency: 3000 MHz
Memory range: 1024 MB
Number of requests: 20
Access pattern: sequential

Initializing external simulator with config: external_simulator_config.yaml
External simulator initialized successfully
Starting simulation...
Issued READ request #1 to addr 0x0 at cycle 1
Issued WRITE request #2 to addr 0x40 at cycle 2
...
Simulation completed in 45623 cycles

=== Mock CPU Statistics ===
Simulation cycles: 45623
Requests issued: 20
Requests completed: 20
Read requests: 14
Write requests: 6
Average latency: 285.50 cycles
Bandwidth: 1.23 GB/s
```

## 🎛️ Configuration Options

### Command Line Parameters
- `--pattern <sequential|random|mixed>`: Memory access pattern
- `--frequency <MHz>`: CPU frequency (default: 3000 MHz)
- `--memory-size <MB>`: Memory range to access (default: 1024 MB)
- `--num-requests <count>`: Number of memory requests (default: 10000)

### Memory Configuration (YAML)
Edit `external_simulator_config.yaml` to configure:
- **DRAM Type**: DDR4, DDR5, HBM, etc.
- **Memory Controller**: Scheduling policies, row policies
- **Timing Parameters**: Clock frequencies, latencies
- **Organization**: Channels, ranks, banks

## 🔧 Key Integration Points

### 1. Request Submission
```cpp
bool success = frontend->receive_external_requests(
    req_type,    // 0=Read, 1=Write
    address,     // Memory address
    source_id,   // Request source
    callback     // Completion callback
);
```

### 2. Simulation Advance
```cpp
memory_system->tick();  // Advance one memory cycle
```

### 3. Request Completion
```cpp
auto callback = [](Ramulator::Request& req) {
    // Handle completed request
    process_completion(req.addr);
};
```

## 📊 Performance Metrics

The simulator automatically collects:
- **Latency**: Average memory access latency
- **Bandwidth**: Achieved memory bandwidth
- **Request Breakdown**: Read vs write statistics
- **Simulation Time**: Total cycles and wall-clock time

## 🛠️ Customization

### Adding New Access Patterns
1. Extend `AccessPattern` enum in `mock_simulator.h`
2. Implement pattern logic in `MockCPU::generate_address()`
3. Add command line parsing in `main.cpp`

### Custom Memory Configurations
1. Copy `external_simulator_config.yaml`
2. Modify DRAM organization and timing parameters
3. Use with `./external_simulator your_config.yaml`

### Integration with Your Simulator
1. Include required Ramulator2 headers
2. Initialize frontend and memory system
3. Submit requests via `receive_external_requests()`
4. Advance simulation with `tick()`

## 📁 File Structure

```
external_simulator_example/
├── mock_simulator.h           # Mock CPU simulator header
├── mock_simulator.cpp         # Mock CPU implementation
├── external_frontend.cpp      # External simulator frontend
├── main.cpp                   # Application entry point
├── CMakeLists.txt            # CMake build configuration
├── Makefile                  # Alternative build system
├── external_simulator_config.yaml  # Memory system configuration
├── run_example.sh            # Demo script
├── README.md                 # Comprehensive documentation
└── USAGE.md                  # This quick start guide
```

## ✅ Verification

If everything works correctly, you should see:
- ✓ Successful Ramulator2 initialization
- ✓ Memory requests being issued and completed
- ✓ Cycle-accurate timing simulation
- ✓ Performance statistics at completion
- ✓ No errors or crashes during execution

## 📚 Next Steps

1. **Experiment** with different access patterns and configurations
2. **Integrate** Ramulator2 into your own simulator using this as a template
3. **Optimize** performance by adjusting request generation patterns
4. **Explore** advanced Ramulator2 features like RowHammer mitigation

For detailed documentation, see `README.md`.
For Ramulator2 configuration options, see the main project documentation.