// SPDX-License-Identifier: MIT
#pragma once
#include <yaml-cpp/yaml.h>
#include "ramulator/base/config_node.h"
namespace Ramulator::Bambu {
// Native 2.1 input is returned unchanged. Legacy Bambu 2.0 presets are expanded
// using data generated from the pinned upstream definitions at build time.
ConfigNode factory_configuration(const YAML::Node& input);
YAML::Node native_configuration(const YAML::Node& input);
}
