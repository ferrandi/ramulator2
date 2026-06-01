#include <vector>
#include <cstdio>

#include "base/base.h"
#include "base/simple_logging.h"
#include "dram_controller/controller.h"
#include "dram_controller/scheduler.h"

namespace Ramulator {

class FRFCFS : public IScheduler, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IScheduler, FRFCFS, "FRFCFS", "FRFCFS DRAM Scheduler.")
  private:
    IDRAM* m_dram;

  public:
    void init() override { };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      m_dram = cast_parent<IDRAMController>()->m_dram;
    };

    ReqBuffer::iterator compare(ReqBuffer::iterator req1, ReqBuffer::iterator req2) override {
      bool ready1 = m_dram->check_ready(req1->command, req1->addr_vec);
      bool ready2 = m_dram->check_ready(req2->command, req2->addr_vec);

      if (ready1 ^ ready2) {
        if (ready1) {
          return req1;
        } else {
          return req2;
        }
      }

      // Fallback to FCFS
      if (req1->arrive <= req2->arrive) {
        return req1;
      } else {
        return req2;
      }
    }

    ReqBuffer::iterator get_best_request(ReqBuffer& buffer) override {
      RAMULATOR_DBG_PRINT("[SCHEDULER] get_best_request called, buffer size=%zu\n", buffer.size());
      if (buffer.size() == 0) {
        return buffer.end();
      }

      RAMULATOR_DBG_PRINT("[SCHEDULER] m_dram pointer: %p\n", (void*)m_dram);

      int req_idx = 0;
      for (auto& req : buffer) {
        RAMULATOR_DBG_PRINT("[SCHEDULER] Processing req %d: addr=0x%llx, final_command=%d, addr_vec.size()=%zu\n",
               req_idx, req.addr, req.final_command, req.addr_vec.size());
        for (size_t i = 0; i < req.addr_vec.size(); i++) {
          RAMULATOR_DBG_PRINT("[SCHEDULER]   addr_vec[%zu]=%d\n", i, req.addr_vec[i]);
        }
        RAMULATOR_DBG_PRINT("[SCHEDULER] About to call get_preq_command\n");
        req.command = m_dram->get_preq_command(req.final_command, req.addr_vec);
        RAMULATOR_DBG_PRINT("[SCHEDULER] get_preq_command returned %d\n", req.command);
        req_idx++;
      }

      auto candidate = buffer.begin();
      for (auto next = std::next(buffer.begin(), 1); next != buffer.end(); next++) {
        candidate = compare(candidate, next);
      }
      RAMULATOR_DBG_PRINT("[SCHEDULER] get_best_request returning\n");
      return candidate;
    }
};

}       // namespace Ramulator
