#include "mock_simulator.h"
#include <iostream>
#include <iomanip>
#include <cassert>

namespace MockSimulator {

MockCPU::MockCPU(uint64_t freq_mhz, uint64_t cache_line_size, AccessPattern pattern,
                 uint64_t memory_range_mb, uint64_t num_requests)
    : m_frequency_mhz(freq_mhz)
    , m_cache_line_size(cache_line_size)
    , m_access_pattern(pattern)
    , m_memory_range_mb(memory_range_mb)
    , m_num_requests(num_requests)
    , m_cycle(0)
    , m_requests_issued(0)
    , m_requests_completed(0)
    , m_frontend(nullptr)
    , m_memory_system(nullptr)
    , m_total_latency(0)
    , m_read_count(0)
    , m_write_count(0)
    , m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
    , m_addr_dist(0, (memory_range_mb * 1024 * 1024) - cache_line_size)
    , m_write_prob(0.0, 1.0)
{
}

void MockCPU::connect_ramulator(Ramulator::IFrontEnd* frontend, Ramulator::IMemorySystem* memory_system) {
    m_frontend = frontend;
    m_memory_system = memory_system;
}

void MockCPU::reset() {
    m_cycle = 0;
    m_requests_issued = 0;
    m_requests_completed = 0;
    m_total_latency = 0;
    m_read_count = 0;
    m_write_count = 0;
    while (!m_pending_transactions.empty()) {
        m_pending_transactions.pop();
    }
}

void MockCPU::tick() {
    m_cycle++;

    // Issue new request if we haven't reached the limit and have bandwidth
    if (m_requests_issued < m_num_requests && m_pending_transactions.size() < 32) {
        issue_request();
    }

    // Tick the memory system
    if (m_memory_system) {
        m_memory_system->tick();
    }
}

bool MockCPU::is_finished() const {
    return m_requests_completed >= m_num_requests;
}

uint64_t MockCPU::generate_address() {
    uint64_t addr = 0;

    switch (m_access_pattern) {
        case AccessPattern::SEQUENTIAL:
            addr = (m_requests_issued * m_cache_line_size) % (m_memory_range_mb * 1024 * 1024);
            break;

        case AccessPattern::RANDOM:
            addr = m_addr_dist(m_rng);
            addr = (addr / m_cache_line_size) * m_cache_line_size;  // Align to cache line
            break;

        case AccessPattern::MIXED:
            if (m_requests_issued % 4 == 0) {
                // 25% sequential
                addr = (m_requests_issued * m_cache_line_size) % (m_memory_range_mb * 1024 * 1024);
            } else {
                // 75% random
                addr = m_addr_dist(m_rng);
                addr = (addr / m_cache_line_size) * m_cache_line_size;
            }
            break;
    }

    return addr;
}

bool MockCPU::should_write() {
    return m_write_prob(m_rng) < 0.3;  // 30% write probability
}

void MockCPU::issue_request() {
    if (!m_frontend) {
        return;
    }

    uint64_t addr = generate_address();
    bool is_write = should_write();

    // Create transaction record
    MemoryTransaction transaction(addr, is_write, m_cycle);

    // Create callback that captures the transaction by value
    auto callback = [this, transaction](Ramulator::Request& req) mutable {
        transaction.complete_cycle = m_cycle;
        transaction.completed = true;

        uint64_t latency = transaction.complete_cycle - transaction.issue_cycle;
        m_total_latency += latency;
        m_requests_completed++;

        if (transaction.is_write) {
            m_write_count++;
        } else {
            m_read_count++;
        }

        if (m_requests_completed % 1000 == 0) {
            std::cout << "Completed " << m_requests_completed << " requests\n";
        }
    };

    // Try to send request to Ramulator2
    int req_type = is_write ? 1 : 0;  // 0 = Read, 1 = Write
    bool success = m_frontend->receive_external_requests(req_type, addr, 0, callback);

    if (success) {
        m_pending_transactions.push(transaction);
        m_requests_issued++;

        if (is_write) {
            std::cout << "Issued WRITE request #" << m_requests_issued << " to addr 0x"
                      << std::hex << addr << std::dec << " at cycle " << m_cycle << "\n";
        } else {
            std::cout << "Issued READ request #" << m_requests_issued << " to addr 0x"
                      << std::hex << addr << std::dec << " at cycle " << m_cycle << "\n";
        }
    }
}

void MockCPU::print_stats() const {
    std::cout << "\n=== Mock CPU Statistics ===\n";
    std::cout << "Simulation cycles: " << m_cycle << "\n";
    std::cout << "Requests issued: " << m_requests_issued << "\n";
    std::cout << "Requests completed: " << m_requests_completed << "\n";
    std::cout << "Read requests: " << m_read_count << "\n";
    std::cout << "Write requests: " << m_write_count << "\n";

    if (m_requests_completed > 0) {
        double avg_latency = static_cast<double>(m_total_latency) / m_requests_completed;
        std::cout << "Average latency: " << std::fixed << std::setprecision(2)
                  << avg_latency << " cycles\n";

        double bandwidth = get_bandwidth_gbps();
        std::cout << "Bandwidth: " << std::fixed << std::setprecision(2)
                  << bandwidth << " GB/s\n";
    }
    std::cout << "=========================\n\n";
}

double MockCPU::get_average_latency() const {
    if (m_requests_completed == 0) return 0.0;
    return static_cast<double>(m_total_latency) / m_requests_completed;
}

double MockCPU::get_bandwidth_gbps() const {
    if (m_cycle == 0) return 0.0;

    uint64_t bytes_transferred = m_requests_completed * m_cache_line_size;
    double seconds = static_cast<double>(m_cycle) / (m_frequency_mhz * 1e6);
    double gbps = (bytes_transferred / seconds) / 1e9;

    return gbps;
}

ExternalSimulator::ExternalSimulator(const std::string& config_path)
    : m_cpu(nullptr)
    , m_frontend(nullptr)
    , m_memory_system(nullptr)
    , m_config_path(config_path)
{
}

ExternalSimulator::~ExternalSimulator() {
    finalize();
}

bool ExternalSimulator::initialize() {
    try {
        std::cout << "Initializing external simulator with config: " << m_config_path << "\n";

        // Parse Ramulator2 configuration
        YAML::Node config = Ramulator::Config::parse_config_file(m_config_path, {});

        // Create Ramulator2 components
        m_frontend = Ramulator::Factory::create_frontend(config);
        m_memory_system = Ramulator::Factory::create_memory_system(config);

        if (!m_frontend || !m_memory_system) {
            std::cerr << "Failed to create Ramulator2 components\n";
            return false;
        }

        // Connect components
        m_frontend->connect_memory_system(m_memory_system);
        m_memory_system->connect_frontend(m_frontend);

        // Create and configure mock CPU
        m_cpu = std::make_unique<MockCPU>();
        m_cpu->connect_ramulator(m_frontend, m_memory_system);

        std::cout << "External simulator initialized successfully\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error initializing simulator: " << e.what() << "\n";
        return false;
    }
}

void ExternalSimulator::finalize() {
    if (m_cpu) {
        m_cpu->print_stats();
    }

    if (m_frontend) {
        m_frontend->finalize();
    }

    if (m_memory_system) {
        m_memory_system->finalize();
    }
}

void ExternalSimulator::run() {
    if (!m_cpu) {
        std::cerr << "Simulator not initialized\n";
        return;
    }

    std::cout << "Starting simulation...\n";
    m_cpu->reset();

    uint64_t last_print_cycle = 0;
    while (!m_cpu->is_finished()) {
        m_cpu->tick();

        // Print progress every 10000 cycles
        if (m_cpu->get_cycle() - last_print_cycle >= 10000) {
            std::cout << "Cycle " << m_cpu->get_cycle()
                      << ", Issued: " << m_cpu->get_requests_issued()
                      << ", Completed: " << m_cpu->get_requests_completed() << "\n";
            last_print_cycle = m_cpu->get_cycle();
        }
    }

    std::cout << "Simulation completed in " << m_cpu->get_cycle() << " cycles\n";
}

void ExternalSimulator::run_cycles(uint64_t num_cycles) {
    if (!m_cpu) {
        std::cerr << "Simulator not initialized\n";
        return;
    }

    uint64_t start_cycle = m_cpu->get_cycle();
    while (m_cpu->get_cycle() - start_cycle < num_cycles && !m_cpu->is_finished()) {
        m_cpu->tick();
    }
}

void ExternalSimulator::configure_cpu(uint64_t freq_mhz, AccessPattern pattern,
                                      uint64_t memory_range_mb, uint64_t num_requests) {
    m_cpu = std::make_unique<MockCPU>(freq_mhz, 64, pattern, memory_range_mb, num_requests);
    if (m_frontend && m_memory_system) {
        m_cpu->connect_ramulator(m_frontend, m_memory_system);
    }
}

void ExternalSimulator::print_final_stats() const {
    if (m_cpu) {
        m_cpu->print_stats();
    }
}

}  // namespace MockSimulator