#include "mock_simulator.h"
#include <iostream>
#include <string>
#include <chrono>

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <ramulator_config.yaml> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --pattern <sequential|random|mixed>    Access pattern (default: mixed)\n";
    std::cout << "  --frequency <MHz>                       CPU frequency in MHz (default: 3000)\n";
    std::cout << "  --memory-size <MB>                      Memory range in MB (default: 1024)\n";
    std::cout << "  --num-requests <count>                  Number of requests to issue (default: 10000)\n";
    std::cout << "  --help                                  Show this help message\n\n";
    std::cout << "Example:\n";
    std::cout << "  " << program_name << " example_config.yaml --pattern random --num-requests 50000\n";
}

MockSimulator::AccessPattern parse_pattern(const std::string& pattern_str) {
    if (pattern_str == "sequential") return MockSimulator::AccessPattern::SEQUENTIAL;
    if (pattern_str == "random") return MockSimulator::AccessPattern::RANDOM;
    if (pattern_str == "mixed") return MockSimulator::AccessPattern::MIXED;

    std::cerr << "Unknown access pattern: " << pattern_str << ". Using mixed.\n";
    return MockSimulator::AccessPattern::MIXED;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string config_path = argv[1];

    // Default parameters
    MockSimulator::AccessPattern pattern = MockSimulator::AccessPattern::MIXED;
    uint64_t frequency_mhz = 3000;
    uint64_t memory_size_mb = 1024;
    uint64_t num_requests = 10000;

    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--pattern" && i + 1 < argc) {
            pattern = parse_pattern(argv[++i]);
        } else if (arg == "--frequency" && i + 1 < argc) {
            frequency_mhz = std::stoull(argv[++i]);
        } else if (arg == "--memory-size" && i + 1 < argc) {
            memory_size_mb = std::stoull(argv[++i]);
        } else if (arg == "--num-requests" && i + 1 < argc) {
            num_requests = std::stoull(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    std::cout << "=== External Simulator with Ramulator2 ===\n";
    std::cout << "Configuration file: " << config_path << "\n";
    std::cout << "CPU frequency: " << frequency_mhz << " MHz\n";
    std::cout << "Memory range: " << memory_size_mb << " MB\n";
    std::cout << "Number of requests: " << num_requests << "\n";
    std::cout << "Access pattern: ";

    switch (pattern) {
        case MockSimulator::AccessPattern::SEQUENTIAL:
            std::cout << "sequential\n";
            break;
        case MockSimulator::AccessPattern::RANDOM:
            std::cout << "random\n";
            break;
        case MockSimulator::AccessPattern::MIXED:
            std::cout << "mixed\n";
            break;
    }
    std::cout << "==========================================\n\n";

    try {
        // Create and initialize the simulator
        MockSimulator::ExternalSimulator simulator(config_path);

        if (!simulator.initialize()) {
            std::cerr << "Failed to initialize simulator\n";
            return 1;
        }

        // Configure the CPU with command line parameters
        simulator.configure_cpu(frequency_mhz, pattern, memory_size_mb, num_requests);

        // Record start time
        auto start_time = std::chrono::high_resolution_clock::now();

        // Run the simulation
        simulator.run();

        // Record end time
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << "\nSimulation completed in " << duration.count() << " ms\n";

        // Print final statistics
        simulator.print_final_stats();

    } catch (const std::exception& e) {
        std::cerr << "Error during simulation: " << e.what() << "\n";
        return 1;
    }

    return 0;
}