#include <iostream>
#include <vector>
#include <map>

// Ramulator2 includes
#include "base/config.h"
#include "frontend/frontend.h"
#include "memory_system/memory_system.h"

int main() {
    try {
        std::cout << "=== Write Request Debug Test ===\n";

        // Initialize Ramulator2
        YAML::Node config = Ramulator::Config::parse_config_file("config.yaml", {});
        auto frontend = Ramulator::Factory::create_frontend(config);
        auto memory_system = Ramulator::Factory::create_memory_system(config);

        frontend->connect_memory_system(memory_system);
        memory_system->connect_frontend(frontend);

        std::cout << "Ramulator2 initialized\n";

        // Test 1: Issue only READ requests
        std::cout << "\nTest 1: READ-only requests\n";
        int completed_reads = 0;
        for (int i = 0; i < 3; i++) {
            uint64_t addr = i * 0x1000;
            auto callback = [&completed_reads, addr](Ramulator::Request& req) {
                std::cout << "READ to 0x" << std::hex << addr << std::dec << " completed\n";
                completed_reads++;
            };

            bool success = frontend->receive_external_requests(0, addr, 0, callback);
            std::cout << "Issued READ to 0x" << std::hex << addr << std::dec
                      << (success ? " SUCCESS" : " FAILED") << "\n";
        }

        // Run simulation for reads
        for (int cycle = 0; cycle < 10000; cycle++) {
            memory_system->tick();
            if (completed_reads == 3) break;
        }
        std::cout << "Completed reads: " << completed_reads << "/3\n";

        // Test 2: Issue only WRITE requests
        std::cout << "\nTest 2: WRITE-only requests\n";
        int completed_writes = 0;
        for (int i = 0; i < 3; i++) {
            uint64_t addr = (i + 10) * 0x1000;
            auto callback = [&completed_writes, addr](Ramulator::Request& req) {
                std::cout << "WRITE to 0x" << std::hex << addr << std::dec << " completed\n";
                completed_writes++;
            };

            bool success = frontend->receive_external_requests(1, addr, 0, callback);
            std::cout << "Issued WRITE to 0x" << std::hex << addr << std::dec
                      << (success ? " SUCCESS" : " FAILED") << "\n";
        }

        // Run simulation for writes
        for (int cycle = 0; cycle < 50000; cycle++) {
            memory_system->tick();
            if (cycle % 10000 == 0) {
                std::cout << "Cycle " << cycle << ": Completed writes: " << completed_writes << "/3\n";
            }
            if (completed_writes == 3) break;
        }

        std::cout << "\nFinal results:\n";
        std::cout << "Completed reads: " << completed_reads << "/3\n";
        std::cout << "Completed writes: " << completed_writes << "/3\n";

        frontend->finalize();
        memory_system->finalize();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}