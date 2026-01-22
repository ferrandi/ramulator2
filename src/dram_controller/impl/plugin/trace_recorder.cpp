#include <vector>
#include <unordered_map>
#include <limits>
#include <filesystem>

#include "base/simple_logging.h"



#include "base/base.h"
#include "dram_controller/controller.h"
#include "dram_controller/plugin.h"

namespace Ramulator {

class TraceRecorder : public IControllerPlugin, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IControllerPlugin, TraceRecorder, "TraceRecorder", "CounterBasedTRR.")
  private:
    IDRAM* m_dram;

    std::filesystem::path m_trace_path; 
    Logger_t m_tracer;

    Clk_t m_clk = 0;

  public:
    void init() override { 
      m_trace_path = param<std::string>("path").desc("Path to the trace file").required();
      auto parent_path = m_trace_path.parent_path();
      std::filesystem::create_directories(parent_path);
      if (!(std::filesystem::exists(parent_path) && std::filesystem::is_directory(parent_path))) {
        throw ConfigurationError("Invalid path to trace file: {}", parent_path.string());
      }
    };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      m_ctrl = cast_parent<IDRAMController>();
      m_dram = m_ctrl->m_dram;

      // Simplified logger setup
      m_tracer = std::make_shared<SimpleLogger>("trace_recorder_ch" + std::to_string(m_ctrl->m_channel_id));      
    };

    void update(bool request_found, ReqBuffer::iterator& req_it) override {
      m_clk++;

      if (request_found) {
        // Simple trace output
        std::string addr_str = fmt::join(req_it->addr_vec, ", ");
        m_tracer->debug("{}, {}, {}", m_clk, m_dram->m_commands(req_it->command), addr_str);
      }

    };

};

}       // namespace Ramulator
