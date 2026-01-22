#ifndef MOCK_SIMULATOR_H
#define MOCK_SIMULATOR_H

#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <random>
#include <chrono>

// Ramulator2 includes
#include "base/config.h"
#include "frontend/frontend.h"
#include "memory_system/memory_system.h"
#include "base/request.h"

namespace MockSimulator {

enum class AccessPattern {
    SEQUENTIAL,
    RANDOM,
    MIXED
};

struct MemoryTransaction {
    uint64_t addr;
    bool is_write;
    uint64_t issue_cycle;
    uint64_t complete_cycle;
    bool completed;

    MemoryTransaction(uint64_t a, bool w, uint64_t cycle)
        : addr(a), is_write(w), issue_cycle(cycle), complete_cycle(0), completed(false) {}
};

class MockCPU {
private:
    // Configuration
    uint64_t m_frequency_mhz;
    uint64_t m_cache_line_size;
    AccessPattern m_access_pattern;
    uint64_t m_memory_range_mb;
    uint64_t m_num_requests;

    // Simulation state
    uint64_t m_cycle;
    uint64_t m_requests_issued;
    uint64_t m_requests_completed;
    std::queue<MemoryTransaction> m_pending_transactions;

    // Random number generation
    std::mt19937 m_rng;
    std::uniform_int_distribution<uint64_t> m_addr_dist;
    std::uniform_real_distribution<double> m_write_prob;

    // Ramulator2 interfaces
    Ramulator::IFrontEnd* m_frontend;
    Ramulator::IMemorySystem* m_memory_system;

    // Statistics
    uint64_t m_total_latency;
    uint64_t m_read_count;
    uint64_t m_write_count;

public:
    MockCPU(uint64_t freq_mhz = 3000,
            uint64_t cache_line_size = 64,
            AccessPattern pattern = AccessPattern::MIXED,
            uint64_t memory_range_mb = 1024,
            uint64_t num_requests = 10000);

    // Ramulator2 integration
    void connect_ramulator(Ramulator::IFrontEnd* frontend, Ramulator::IMemorySystem* memory_system);

    // Simulation control
    void reset();
    void tick();
    bool is_finished() const;

    // Request generation
    uint64_t generate_address();
    bool should_write();
    void issue_request();

    // Callback for completed requests
    void on_request_complete(Ramulator::Request& req);

    // Statistics
    void print_stats() const;
    double get_average_latency() const;
    double get_bandwidth_gbps() const;

    // Getters
    uint64_t get_cycle() const { return m_cycle; }
    uint64_t get_requests_issued() const { return m_requests_issued; }
    uint64_t get_requests_completed() const { return m_requests_completed; }
};

class ExternalSimulator {
private:
    std::unique_ptr<MockCPU> m_cpu;
    Ramulator::IFrontEnd* m_frontend;
    Ramulator::IMemorySystem* m_memory_system;
    std::string m_config_path;

public:
    ExternalSimulator(const std::string& config_path);
    ~ExternalSimulator();

    // Initialization
    bool initialize();
    void finalize();

    // Simulation control
    void run();
    void run_cycles(uint64_t num_cycles);

    // Configuration
    void configure_cpu(uint64_t freq_mhz, AccessPattern pattern, uint64_t memory_range_mb, uint64_t num_requests);

    // Statistics
    void print_final_stats() const;
};

}  // namespace MockSimulator

#endif // MOCK_SIMULATOR_H