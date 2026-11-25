// SPDX-License-Identifier: MIT
#include "ramulator/base/config.h"
#include "ramulator/frontend/i_frontend.h"
#include "ramulator/memory_system/i_memory_system.h"
#include <iostream>
#include <memory>
#include <stdexcept>
int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--help") {
    std::cout << "Ramulator 2.1: ramulator2 -f <native-config.yaml>\n"; return 0;
  }
  if (argc != 3 || std::string(argv[1]) != "-f") return 2;
  try {
    auto config = Ramulator::Config::parse_config_file(argv[2]);
    std::unique_ptr<Ramulator::IFrontEnd> frontend(Ramulator::Factory::create_frontend(config));
    std::unique_ptr<Ramulator::IMemorySystem> memory(Ramulator::Factory::create_memory_system(config));
    frontend->connect_memory_system(memory.get()); memory->connect_frontend(frontend.get());
    const int fe_tick = frontend->get_clock_ratio(), mem_tick = memory->get_clock_ratio();
    if (fe_tick <= 0 || mem_tick <= 0) throw std::runtime_error("clock_ratio must be positive");
    int fe_count = mem_tick - 1, mem_count = fe_tick - 1;
    for (;;) {
      if (++fe_count >= mem_tick) { fe_count = 0; frontend->tick(); }
      if (frontend->is_finished()) break;
      if (++mem_count >= fe_tick) { mem_count = 0; memory->tick(); }
    }
    memory->finalize(); memory->update_stats_recursive(); memory->print_stats(std::cout);
  } catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }
}
