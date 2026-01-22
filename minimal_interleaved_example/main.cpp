#include <iostream>
#include <vector>
#include <map>

// Ramulator2 includes
#include "base/config.h"
#include "frontend/frontend.h"
#include "memory_system/memory_system.h"

struct PendingRequest {
    uint64_t addr;
    bool is_write;
    int id;
    uint64_t issue_cycle;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <config.yaml>\n";
        return 1;
    }

    std::string config_path = argv[1];

    try {
        std::cout << "Loading config: " << config_path << "\n";

        // 1. Initialize Ramulator2
        YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});
        auto frontend = Ramulator::Factory::create_frontend(config);
        auto memory_system = Ramulator::Factory::create_memory_system(config);

        if (!frontend || !memory_system) {
            std::cerr << "Failed to create Ramulator2 components\n";
            return 1;
        }

        frontend->connect_memory_system(memory_system);
        memory_system->connect_frontend(frontend);
        std::cout << "Ramulator2 initialized successfully\n\n";

        // 2. Simulation state
        std::map<int, PendingRequest> pending_requests;  // Track by request ID
        int completed_requests = 0;
        int total_requests_to_issue = 10;
        int next_request_id = 1;
        uint64_t current_cycle = 0;
        uint64_t next_request_cycle = 1;  // Issue first request at cycle 1
        uint64_t request_interval = 500;  // Issue new request every 500 cycles

        // Prepare addresses to request
        std::vector<uint64_t> addresses = {
            0x0, 0x1000, 0x2000, 0x3000, 0x4000,
            0x5000, 0x6000, 0x7000, 0x8000, 0x9000
        };

        std::cout << "Starting interleaved simulation...\n";
        std::cout << "Will issue " << total_requests_to_issue << " requests with "
                  << request_interval << " cycle intervals\n\n";

        // 3. Main simulation loop
        while (completed_requests < total_requests_to_issue || !pending_requests.empty()) {
            current_cycle++;

            // Try to issue a new request if it's time and we have more to issue
            if (current_cycle >= next_request_cycle &&
                next_request_id <= total_requests_to_issue) {

                uint64_t addr = addresses[(next_request_id - 1) % addresses.size()];
                bool is_write = (next_request_id % 3 == 0);  // Every 3rd request is write

                // Create completion callback that removes from pending requests
                int request_id = next_request_id;
                auto callback = [&pending_requests, &completed_requests, request_id, addr, is_write]
                               (Ramulator::Request& req) {
                    // Remove from pending requests map
                    auto it = pending_requests.find(request_id);
                    if (it != pending_requests.end()) {
                        uint64_t latency = req.depart - it->second.issue_cycle;
                        std::cout << "Completed " << (is_write ? "WRITE" : "READ")
                                  << " request #" << request_id
                                  << " to addr 0x" << std::hex << addr << std::dec
                                  << " (latency: " << latency << " cycles)\n";
                        pending_requests.erase(it);
                        completed_requests++;
                    }
                };

                // Try to submit the request
                int req_type = is_write ? 1 : 0;
                bool success = frontend->receive_external_requests(req_type, addr, 0, callback);

                if (success) {
                    std::cout << "Cycle " << current_cycle << ": Issued "
                              << (is_write ? "WRITE" : "READ") << " request #" << next_request_id
                              << " to addr 0x" << std::hex << addr << std::dec << "\n";

                    // Add to pending requests map for tracking
                    pending_requests[next_request_id] = {addr, is_write, next_request_id, current_cycle};
                    next_request_id++;
                    next_request_cycle = current_cycle + request_interval;
                } else {
                    std::cout << "Cycle " << current_cycle << ": Request #" << next_request_id
                              << " BLOCKED (Ramulator2 buffers full), will retry\n";
                    // Don't increment next_request_id, try again next cycle
                    next_request_cycle = current_cycle + 1;  // Retry soon
                }
            }

            // Always advance the memory system
            memory_system->tick();

            // Print progress every 5000 cycles
            if (current_cycle % 5000 == 0) {
                std::cout << "Cycle " << current_cycle << ": Issued "
                          << (next_request_id - 1) << "/" << total_requests_to_issue
                          << ", Completed " << completed_requests
                          << ", Pending " << pending_requests.size() << "\n";
            }

            // Safety limit
            if (current_cycle > 500000) {
                std::cout << "Reached cycle limit, stopping simulation\n";
                break;
            }
        }

        // 4. Print results
        std::cout << "\n=== Simulation Results ===\n";
        std::cout << "Total cycles: " << current_cycle << "\n";
        std::cout << "Requests issued: " << (next_request_id - 1) << "/" << total_requests_to_issue << "\n";
        std::cout << "Requests completed: " << completed_requests << "\n";
        std::cout << "Requests still pending: " << pending_requests.size() << "\n";

        if (completed_requests == total_requests_to_issue) {
            std::cout << "All requests completed successfully!\n";
        } else {
            std::cout << "Some requests did not complete\n";
        }

        // 5. Cleanup
        frontend->finalize();
        memory_system->finalize();

        std::cout << "\n=== Example completed ===\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}