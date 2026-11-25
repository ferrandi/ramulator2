// SPDX-License-Identifier: MIT
#include "ramulator/bambu/legacy_config.h"
#include "ramulator/base/config.h"
#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
namespace Ramulator::Bambu {
ConfigNode factory_configuration(const YAML::Node& input) {
  return Config::parse_config_string(YAML::Dump(native_configuration(input)));
}

#include "legacy_catalog.inc"
YAML::Node native_configuration(const YAML::Node& input) {
  if (input["memory_system"]) return YAML::Clone(input);
  auto old = input["MemorySystem"];
  if (!old) throw std::invalid_argument("Missing Ramulator memory_system configuration");
  const auto impl = old["impl"].as<std::string>();
  if (impl == "DummyMemorySystem")
    throw std::invalid_argument("Ramulator must provide a finite positive DRAM tCK");
  if (impl != "GenericDRAM") throw std::invalid_argument("Unsupported legacy memory system: " + impl);
  auto spec = old["DRAM"];
  const auto standard = spec["impl"].as<std::string>();
  auto timing = spec["timing"]["preset"].as<std::string>();
  if (timing == "HBM2_2Gbps") timing = "HBM2_2000Mbps";
  auto organization = spec["org"]["preset"].as<std::string>();
  // 2.0 names describe physical-channel density; 2.1 uses pseudochannel density.
  if (standard == "HBM2") {
    if (organization == "HBM2_2Gb") organization = "HBM2_1Gb";
    else if (organization == "HBM2_4Gb") organization = "HBM2_2Gb";
    else if (organization == "HBM2_8Gb") organization = "HBM2_4Gb";
  }
  auto key = standard + "|" + organization + "|" + timing;
  const auto entry = legacy_dram_catalog.find(key);
  if (entry == legacy_dram_catalog.end()) throw std::invalid_argument("Unsupported legacy DRAM preset: " + key);
  auto dram = YAML::Load(entry->second);
  // Organization overrides do not alter the upstream timing constraints.
  const std::vector<std::string> levels = standard == "HBM2"
    ? std::vector<std::string>{"pseudochannel", "sid", "bankgroup", "bank", "row", "column"}
    : standard == "DDR3" ? std::vector<std::string>{"rank", "bank", "row", "column"}
    : std::vector<std::string>{"rank", "bankgroup", "bank", "row", "column"};
  for (auto kv : spec["org"]) {
    auto name = kv.first.as<std::string>();
    if (name == "preset" || name == "channel") continue;
    auto at = std::find(levels.begin(), levels.end(), name);
    if (at == levels.end()) throw std::invalid_argument("Unsupported legacy organization override: " + name);
    dram["org"]["count"][1 + std::distance(levels.begin(), at)] = kv.second;
  }
  for (auto kv : spec["timing"]) {
    if (kv.first.as<std::string>() != "preset")
      throw std::invalid_argument("Legacy timing overrides require a native 2.1 YAML with resolved constraints");
  }
  auto channels = spec["org"]["channel"].as<unsigned>(1);
  if (!channels || channels > 256) throw std::invalid_argument("Invalid legacy channel count");
  YAML::Node result;
  result["frontend"]["impl"] = "External";
  result["frontend"]["clock_ratio"] = 1;
  auto system = result["memory_system"];
  system["impl"] = "GenericDRAM";
  system["clock_ratio"] = 1;
  system["channel_mapper"]["impl"] = "CacheLineInterleave";
  YAML::Node controller;
  controller["impl"] = standard == "HBM2" ? "HBM12" : "GenericDDR";
  controller["dram"] = dram;
  auto old_controller = old["Controller"];
  if (old_controller["impl"].as<std::string>("Generic") != "Generic")
    throw std::invalid_argument("Legacy controller requires explicit native 2.1 configuration");
  for (auto field : {"wr_low_watermark", "wr_high_watermark"})
    if (old_controller[field]) controller[field] = old_controller[field];
  controller["scheduler"]["impl"] = old_controller["Scheduler"]["impl"].as<std::string>("FRFCFS");
  controller["refresh_manager"]["impl"] = old_controller["RefreshManager"]["impl"].as<std::string>("AllBank");
  auto policy = old_controller["RowPolicy"]["impl"].as<std::string>("OpenRowPolicy");
  if (policy != "OpenRowPolicy" && policy != "ClosedRowPolicy")
    throw std::invalid_argument("Unsupported legacy row policy: " + policy);
  controller["row_policy"]["impl"] = policy == "OpenRowPolicy" ? "Open" : "ClosedCAP";
  if (policy == "ClosedRowPolicy") controller["row_policy"]["cap"] = old_controller["RowPolicy"]["cap"].as<int>(4);
  const auto mapper = old["AddrMapper"]["impl"].as<std::string>("RoBaRaCoCh");
  if (mapper != "RoBaRaCoCh" && mapper != "ChRaBaRoCo")
    throw std::invalid_argument("Legacy mapper requires explicit native 2.1 channel mapping: " + mapper);
  controller["addr_mapper"]["impl"] = mapper;
  if (mapper == "ChRaBaRoCo") {
    // Put channel bits above every intra-channel coordinate, as in 2.0.
    unsigned mapped_bits = 0;
    for (size_t level = 1; level < dram["org"]["count"].size(); ++level) {
      auto count = dram["org"]["count"][level].as<unsigned>();
      if (level + 1 == dram["org"]["count"].size()) count /= standard == "HBM2" ? 4 : 8;
      if (!count || (count & (count - 1))) throw std::invalid_argument("Non-power-of-two legacy organization");
      while (count > 1) { count >>= 1; ++mapped_bits; }
    }
    system["channel_mapper"]["interleave_bits"] = mapped_bits;
  }
  if (old_controller["plugins"] && old_controller["plugins"].size())
    throw std::invalid_argument("Legacy controller plugins require native 2.1 configuration");
  system["controllers"] = YAML::Node(YAML::NodeType::Sequence);
  for (unsigned i = 0; i < channels; ++i) system["controllers"].push_back(YAML::Clone(controller));
  return result;
}
}
