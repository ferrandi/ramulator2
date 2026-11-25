# SPDX-License-Identifier: MIT
"""Build-time compatibility data from the pinned upstream DRAM definitions."""
import json
import sys
from pathlib import Path
import ramulator

out = ['// Generated from upstream Python DRAM definitions; do not edit.',
       'static const std::map<std::string, const char*> legacy_dram_catalog = {']
for name in ('DDR3', 'DDR4', 'DDR5', 'HBM2'):
    cls = getattr(ramulator.dram, name)
    for org in cls.org_presets:
        for timing in cls.timing_presets:
            config = cls(org_preset=org, timing_preset=timing).to_config()
            key = '|'.join((name, org, timing))
            out.append('{' + json.dumps(key) + ', R"RAM(' + json.dumps(config, separators=(',', ':')) + ')RAM"},')
out.append('};')
Path(sys.argv[1]).write_text('\n'.join(out) + '\n')
