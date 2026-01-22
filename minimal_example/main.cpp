#include <iostream>
#include <vector>

// Ramulator2 includes
#include "base/config.h"
#include "frontend/frontend.h"
#include "memory_system/memory_system.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <config.yaml>\n";
        return 1;
    }

    std::string config_path = argv[1];

    try {
        std::cout << "=== Minimal Ramulator2 Integration Example ===\n";
        std::cout << "Loading config: " << config_path << "\n";

        // 1. Parse Ramulator2 configuration
        YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});

        // 2. Create Ramulator2 components
        auto frontend = Ramulator::Factory::create_frontend(config);
        auto memory_system = Ramulator::Factory::create_memory_system(config);

        if (!frontend || !memory_system) {
            std::cerr << "Failed to create Ramulator2 components\n";
            return 1;
        }

        // 3. Connect components
        frontend->connect_memory_system(memory_system);
        memory_system->connect_frontend(frontend);

        std::cout << "Ramulator2 initialized successfully\n";

        // 4. Issue a few simple memory requests
        std::vector<uint64_t> addresses = {0x0, 0x1000, 0x2000, 0x3000, 0x4000};
        int completed_requests = 0;
        int total_requests = addresses.size();

        for (size_t i = 0; i < addresses.size(); i++) {
            uint64_t addr = addresses[i];
            bool is_write = (i % 3 == 0);  // Every 3rd request is a write

            // Create completion callback
            auto callback = [&completed_requests, addr, is_write](Ramulator::Request& req) {
                std::cout << "Completed " << (is_write ? "WRITE" : "READ")
                          << " to address 0x" << std::hex << addr << std::dec << "\n";
                completed_requests++;
            };

            // Submit request to Ramulator2
            int req_type = is_write ? 1 : 0;  // 0=Read, 1=Write
            bool success = frontend->receive_external_requests(req_type, addr, 0, callback);

            if (success) {
                std::cout << "Issued " << (is_write ? "WRITE" : "READ")
                          << " request to address 0x" << std::hex << addr << std::dec << "\n";
            } else {
                std::cout << "Failed to issue request to address 0x" << std::hex << addr << std::dec << "\n";
            }
        }

        // 5. Run simulation until all requests complete
        std::cout << "Running simulation...\n";
        int cycles = 0;
        const int max_cycles = 100000;  // Safety limit

        while (completed_requests < total_requests && cycles < max_cycles) {
            memory_system->tick();
            cycles++;

            // Print progress every 10000 cycles
            if (cycles % 10000 == 0) {
                std::cout << "Cycle " << cycles << ": " << completed_requests
                          << "/" << total_requests << " requests completed\n";
            }
        }

        // 6. Print results
        std::cout << "\n=== Simulation Results ===\n";
        std::cout << "Total cycles: " << cycles << "\n";
        std::cout << "Completed requests: " << completed_requests << "/" << total_requests << "\n";

        if (completed_requests == total_requests) {
            std::cout << "All requests completed successfully!\n";
        } else {
            std::cout << "Some requests did not complete\n";
        }

        // 7. Cleanup
        frontend->finalize();
        memory_system->finalize();

        std::cout << "=== Example completed ===\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}