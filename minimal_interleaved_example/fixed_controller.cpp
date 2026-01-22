#include "dram_controller/controller.h"
#include "memory_system/memory_system.h"

namespace Ramulator {

class FixedGenericDRAMController final : public IDRAMController, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IDRAMController, FixedGenericDRAMController, "FixedGeneric", "A generic DRAM controller with proper write callback handling.");
  private:
    std::deque<Request> pending_reads;    // Queue for read requests that are about to finish
    std::deque<Request> pending_writes;   // Queue for write requests that are about to finish

    ReqBuffer m_active_buffer;            // Buffer for requests being served
    ReqBuffer m_priority_buffer;          // Buffer for high-priority requests
    ReqBuffer m_read_buffer;              // Read request buffer
    ReqBuffer m_write_buffer;             // Write request buffer

    int m_bank_addr_idx = -1;
    float m_wr_low_watermark;
    float m_wr_high_watermark;
    bool  m_is_write_mode = false;

    // Statistics
    size_t s_row_hits = 0;
    size_t s_row_misses = 0;
    size_t s_row_conflicts = 0;
    size_t s_read_row_hits = 0;
    size_t s_read_row_misses = 0;
    size_t s_read_row_conflicts = 0;
    size_t s_write_row_hits = 0;
    size_t s_write_row_misses = 0;
    size_t s_write_row_conflicts = 0;

    size_t m_num_cores = 0;
    std::vector<size_t> s_read_row_hits_per_core;
    std::vector<size_t> s_read_row_misses_per_core;
    std::vector<size_t> s_read_row_conflicts_per_core;

    size_t s_num_read_reqs = 0;
    size_t s_num_write_reqs = 0;
    size_t s_num_other_reqs = 0;
    size_t s_queue_len = 0;
    size_t s_read_queue_len = 0;
    size_t s_write_queue_len = 0;
    size_t s_priority_queue_len = 0;
    float s_queue_len_avg = 0;
    float s_read_queue_len_avg = 0;
    float s_write_queue_len_avg = 0;
    float s_priority_queue_len_avg = 0;

    size_t s_read_latency = 0;
    size_t s_write_latency = 0;  // Add write latency tracking
    float s_avg_read_latency = 0;
    float s_avg_write_latency = 0;  // Add average write latency

  public:
    void init() override {
      m_wr_low_watermark =  param<float>("wr_low_watermark").desc("Threshold for switching back to read mode.").default_val(0.2f);
      m_wr_high_watermark = param<float>("wr_high_watermark").desc("Threshold for switching to write mode.").default_val(0.8f);

      m_scheduler = create_child_ifce<IScheduler>();
      m_refresh = create_child_ifce<IRefreshManager>();
      m_rowpolicy = create_child_ifce<IRowPolicy>();

      if (m_config["plugins"]) {
        YAML::Node plugin_configs = m_config["plugins"];
        for (YAML::iterator it = plugin_configs.begin(); it != plugin_configs.end(); ++it) {
          m_plugins.push_back(create_child_ifce<IControllerPlugin>(*it));
        }
      }
    };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      m_dram = memory_system->get_ifce<IDRAM>();
      m_bank_addr_idx = m_dram->m_levels("bank");
      m_priority_buffer.max_size = 512*3 + 32;

      m_num_cores = frontend->get_num_cores();

      s_read_row_hits_per_core.resize(m_num_cores, 0);
      s_read_row_misses_per_core.resize(m_num_cores, 0);
      s_read_row_conflicts_per_core.resize(m_num_cores, 0);

      // Register all statistics
      register_stat(s_row_hits).name("row_hits_{}", m_channel_id);
      register_stat(s_row_misses).name("row_misses_{}", m_channel_id);
      register_stat(s_row_conflicts).name("row_conflicts_{}", m_channel_id);
      register_stat(s_read_row_hits).name("read_row_hits_{}", m_channel_id);
      register_stat(s_read_row_misses).name("read_row_misses_{}", m_channel_id);
      register_stat(s_read_row_conflicts).name("read_row_conflicts_{}", m_channel_id);
      register_stat(s_write_row_hits).name("write_row_hits_{}", m_channel_id);
      register_stat(s_write_row_misses).name("write_row_misses_{}", m_channel_id);
      register_stat(s_write_row_conflicts).name("write_row_conflicts_{}", m_channel_id);

      for (size_t core_id = 0; core_id < m_num_cores; core_id++) {
        register_stat(s_read_row_hits_per_core[core_id]).name("read_row_hits_core_{}", core_id);
        register_stat(s_read_row_misses_per_core[core_id]).name("read_row_misses_core_{}", core_id);
        register_stat(s_read_row_conflicts_per_core[core_id]).name("read_row_conflicts_core_{}", core_id);
      }

      register_stat(s_num_read_reqs).name("num_read_reqs_{}", m_channel_id);
      register_stat(s_num_write_reqs).name("num_write_reqs_{}", m_channel_id);
      register_stat(s_num_other_reqs).name("num_other_reqs_{}", m_channel_id);
      register_stat(s_queue_len).name("queue_len_{}", m_channel_id);
      register_stat(s_read_queue_len).name("read_queue_len_{}", m_channel_id);
      register_stat(s_write_queue_len).name("write_queue_len_{}", m_channel_id);
      register_stat(s_priority_queue_len).name("priority_queue_len_{}", m_channel_id);
      register_stat(s_queue_len_avg).name("queue_len_avg_{}", m_channel_id);
      register_stat(s_read_queue_len_avg).name("read_queue_len_avg_{}", m_channel_id);
      register_stat(s_write_queue_len_avg).name("write_queue_len_avg_{}", m_channel_id);
      register_stat(s_priority_queue_len_avg).name("priority_queue_len_avg_{}", m_channel_id);

      register_stat(s_read_latency).name("read_latency_{}", m_channel_id);
      register_stat(s_avg_read_latency).name("avg_read_latency_{}", m_channel_id);
      register_stat(s_write_latency).name("write_latency_{}", m_channel_id);
      register_stat(s_avg_write_latency).name("avg_write_latency_{}", m_channel_id);
    };

    bool send(Request& req) override {
      req.final_command = m_dram->m_request_translations(req.type_id);

      switch (req.type_id) {
        case Request::Type::Read:
          s_num_read_reqs++;
          break;
        case Request::Type::Write:
          s_num_write_reqs++;
          break;
        default:
          s_num_other_reqs++;
          break;
      }

      // Forward existing write requests to incoming read requests
      if (req.type_id == Request::Type::Read) {
        auto compare_addr = [req](const Request& wreq) {
          return wreq.addr == req.addr;
        };
        if (std::find_if(m_write_buffer.begin(), m_write_buffer.end(), compare_addr) != m_write_buffer.end()) {
          req.depart = m_clk + 1;
          pending_reads.push_back(req);
          return true;
        }
      }

      // Enqueue to corresponding buffer
      bool is_success = false;
      req.arrive = m_clk;
      if (req.type_id == Request::Type::Read) {
        is_success = m_read_buffer.enqueue(req);
      } else if (req.type_id == Request::Type::Write) {
        is_success = m_write_buffer.enqueue(req);
      } else {
        throw std::runtime_error("Invalid request type!");
      }

      if (!is_success) {
        req.arrive = -1;
        return false;
      }

      return true;
    };

    bool priority_send(Request& req) override {
      req.final_command = m_dram->m_request_translations(req.type_id);
      return m_priority_buffer.enqueue(req);
    }

    void tick() override {
      m_clk++;

      // Update statistics
      s_queue_len += m_read_buffer.size() + m_write_buffer.size() + m_priority_buffer.size() +
                     pending_reads.size() + pending_writes.size();
      s_read_queue_len += m_read_buffer.size() + pending_reads.size();
      s_write_queue_len += m_write_buffer.size() + pending_writes.size();
      s_priority_queue_len += m_priority_buffer.size();

      // 1. Serve completed requests
      serve_completed_requests();

      m_refresh->tick();

      // 2. Try to find a request to serve
      ReqBuffer::iterator req_it;
      ReqBuffer* buffer = nullptr;
      bool request_found = schedule_request(req_it, buffer);

      // 2.1 Take row policy action
      m_rowpolicy->update(request_found, req_it);

      // 3. Update all plugins
      for (auto plugin : m_plugins) {
        plugin->update(request_found, req_it);
      }

      // 4. Issue commands to serve the request
      if (request_found) {
        if (req_it->is_stat_updated == false) {
          update_request_stats(req_it);
        }
        m_dram->issue_command(req_it->command, req_it->addr_vec);

        // If we are issuing the last command, set depart time and move to pending
        if (req_it->command == req_it->final_command) {
          if (req_it->type_id == Request::Type::Read) {
            req_it->depart = m_clk + m_dram->m_read_latency;
            pending_reads.push_back(*req_it);
          } else if (req_it->type_id == Request::Type::Write) {
            // ✅ FIX: Also handle write completion properly
            req_it->depart = m_clk + m_dram->m_write_latency;
            pending_writes.push_back(*req_it);
          }
          buffer->remove(req_it);
        } else {
          if (m_dram->m_command_meta(req_it->command).is_opening) {
            if (m_active_buffer.enqueue(*req_it)) {
              buffer->remove(req_it);
            }
          }
        }
      }
    };

  private:
    // ✅ FIX: Serve both read and write completions
    void serve_completed_requests() {
      // Serve completed reads
      while (!pending_reads.empty()) {
        auto& req = pending_reads.front();
        if (req.depart <= m_clk) {
          if (req.depart - req.arrive > 1) {
            s_read_latency += req.depart - req.arrive;
          }
          if (req.callback) {
            req.callback(req);
          }
          pending_reads.pop_front();
        } else {
          break;
        }
      }

      // Serve completed writes
      while (!pending_writes.empty()) {
        auto& req = pending_writes.front();
        if (req.depart <= m_clk) {
          if (req.depart - req.arrive > 1) {
            s_write_latency += req.depart - req.arrive;
          }
          if (req.callback) {
            req.callback(req);  // ✅ Call write callbacks!
          }
          pending_writes.pop_front();
        } else {
          break;
        }
      }
    };

    // ... [Include all other helper methods from original controller]
    // [Abbreviated for brevity - would include is_row_hit, update_request_stats,
    //  set_write_mode, schedule_request, etc.]

    void finalize() override {
      if (s_num_read_reqs > 0) {
        s_avg_read_latency = (float) s_read_latency / (float) s_num_read_reqs;
      }
      if (s_num_write_reqs > 0) {
        s_avg_write_latency = (float) s_write_latency / (float) s_num_write_reqs;
      }

      s_queue_len_avg = (float) s_queue_len / (float) m_clk;
      s_read_queue_len_avg = (float) s_read_queue_len / (float) m_clk;
      s_write_queue_len_avg = (float) s_write_queue_len / (float) m_clk;
      s_priority_queue_len_avg = (float) s_priority_queue_len / (float) m_clk;
    }

    // Helper methods would be included here...
    // [Abbreviated for space]
};

}   // namespace Ramulator