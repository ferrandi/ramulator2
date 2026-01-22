# Minimal Interleaved Ramulator2 Example

This example demonstrates the **correct** way to integrate Ramulator2 with an external simulator by interleaving request generation with memory system simulation, rather than issuing all requests upfront.

## Key Differences from Basic Approach

### ❌ Basic (Unrealistic) Approach
```cpp
// Issue all requests first
for (auto addr : addresses) {
    frontend->receive_external_requests(...);
}
// Then simulate
while (not_done) {
    memory_system->tick();
}
```

### ✅ Interleaved (Realistic) Approach
```cpp
while (simulation_running) {
    // Issue new request if it's time and possible
    if (should_issue_request()) {
        bool success = frontend->receive_external_requests(...);
        if (!success) {
            // Handle backpressure - buffers full
        }
    }

    // Always advance memory system
    memory_system->tick();
}
```

## What This Example Shows

1. **Realistic Request Timing**: Requests are issued every 500 cycles (configurable)
2. **Backpressure Handling**: Detects when Ramulator2 buffers are full
3. **Interleaved Simulation**: Memory system advances every cycle
4. **Gradual Completion**: Requests complete at different times during simulation

## Quick Start

```bash
# Build and run
make test

# Or manually
make
LD_LIBRARY_PATH=.. ./minimal_interleaved_example config.yaml
```

## Expected Output

```
=== Minimal Interleaved Ramulator2 Example ===
Starting interleaved simulation...
Will issue 10 requests with 500 cycle intervals

Cycle 1: Issued READ request #1 to addr 0x0
Cycle 1: Completed READ request #1 to addr 0x0
Cycle 501: Issued READ request #2 to addr 0x1000
Cycle 501: Completed READ request #2 to addr 0x1000
Cycle 1001: Issued WRITE request #3 to addr 0x2000
Cycle 1501: Issued READ request #4 to addr 0x3000
...
Cycle 5000: Issued 10/10, Completed 7
```

## Key Code Sections

### 1. Callback Mechanism (CRITICAL)
```cpp
// Callback is invoked AUTOMATICALLY by Ramulator2 when request completes
auto callback = [&pending_requests, &completed_requests, request_id]
               (Ramulator::Request& req) {
    // This runs INSIDE memory_system->tick() when request finishes
    auto it = pending_requests.find(request_id);
    if (it != pending_requests.end()) {
        uint64_t latency = req.depart - it->second.issue_cycle;
        pending_requests.erase(it);  // Remove from our tracking
        completed_requests++;
    }
};

// Pass callback to Ramulator2 - it will call it later
frontend->receive_external_requests(req_type, addr, 0, callback);
```

**Important**: Callbacks are invoked automatically during `memory_system->tick()` when the memory operation completes inside Ramulator2.

### 2. Request Timing Control
```cpp
uint64_t next_request_cycle = 1;
uint64_t request_interval = 500;  // Issue every 500 cycles

if (current_cycle >= next_request_cycle && have_more_requests) {
    // Try to issue request
}
```

### 3. Backpressure Handling
```cpp
bool success = frontend->receive_external_requests(req_type, addr, 0, callback);
if (!success) {
    std::cout << "Request BLOCKED (buffers full), will retry\n";
    next_request_cycle = current_cycle + 1;  // Retry next cycle
}
```

### 4. Interleaved Simulation Loop
```cpp
while (simulation_running) {
    // Try to issue new request (if conditions met)
    if (should_issue_request()) {
        attempt_request_issue();
    }

    // Always advance memory system
    memory_system->tick();
    current_cycle++;
}
```

### 5. Request Tracking Management
```cpp
// Use map to track pending requests by ID
std::map<int, PendingRequest> pending_requests;

// Add when issuing
pending_requests[request_id] = {addr, is_write, request_id, current_cycle};

// Remove in callback when completed (automatic)
pending_requests.erase(request_id);
```

## How Callbacks Work

1. **When Issued**: You pass a callback function to `receive_external_requests()`
2. **Ramulator2 Stores**: Ramulator2 stores your callback with the request
3. **During Simulation**: `memory_system->tick()` processes DRAM timing
4. **Automatic Invocation**: When request completes, Ramulator2 calls your callback
5. **Your Cleanup**: Callback removes request from your tracking and updates stats

**Key Point**: You don't manually "pop" anything. Ramulator2 automatically calls your callback when the memory operation completes.

## ⚠️ Important Issue: Write Request Callbacks

**Current Issue**: The GEM5 frontend and GenericDRAMController have a bug where **write request callbacks are not invoked**.

**Root Cause**: In `/src/dram_controller/impl/generic_dram_controller.cpp` lines 207-213:
```cpp
if (req_it->command == req_it->final_command) {
  if (req_it->type_id == Request::Type::Read) {
    req_it->depart = m_clk + m_dram->m_read_latency;
    pending.push_back(*req_it);  // ✅ READ requests added to pending queue
  } else if (req_it->type_id == Request::Type::Write) {
    // TODO: Add code to update statistics
    // ❌ WRITE requests are NOT added to pending queue!
  }
}
```

**Impact**:
- ✅ READ requests: Callbacks invoked correctly
- ❌ WRITE requests: Callbacks never invoked, causing pending request tracking issues

**Workaround for This Example**:
For demonstration purposes, this example shows the issue but continues running. In a real integration, you would need to:

1. **Patch the controller** to add write requests to pending queue with proper latency
2. **Use only READ requests** for critical synchronization
3. **Implement write acknowledgment** at a higher level

**Expected Fix** (for Ramulator2 developers):
```cpp
} else if (req_it->type_id == Request::Type::Write) {
  // Calculate write latency: nCWL + nBL (similar to read latency)
  req_it->depart = m_clk + m_dram->m_timing_vals("nCWL") + m_dram->m_timing_vals("nBL");
  pending.push_back(*req_it);  // Add to pending queue for callback
}
```

## Integration Benefits

This approach provides:
- **Realistic Timing**: Matches how real simulators work
- **Flow Control**: Proper handling of memory system backpressure
- **Scalability**: Works with any request generation pattern
- **Accuracy**: True cycle-by-cycle co-simulation
- **Proper Tracking**: Correct request lifecycle management

## Comparison with Basic Example

| Aspect | Basic Example | Interleaved Example |
|--------|---------------|-------------------|
| Request Issue | All upfront | During simulation |
| Timing | Unrealistic | Realistic |
| Backpressure | Not handled | Properly handled |
| Simulator Integration | Poor | Excellent |
| Real-world Applicability | Limited | High |

## Files

- `main.cpp` (120 lines) - Complete interleaved simulation example
- `config.yaml` - DDR4 memory configuration
- `Makefile` - Build system with test target

This example provides the foundation for integrating Ramulator2 into any cycle-accurate external simulator.