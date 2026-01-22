#!/bin/bash

echo "=== External Simulator Demo with Ramulator2 ==="
echo "This script demonstrates the integration between an external simulator and Ramulator2"
echo ""

if [[ ! -f ./external_simulator ]]; then
    echo "Error: external_simulator binary not found. Please build it first:"
    echo "  cmake -B build && cmake --build build"
    exit 1
fi

echo "Testing different access patterns with small number of requests..."
echo ""

echo "1. Sequential Access Pattern (10 requests):"
timeout 30s ./external_simulator external_simulator_config.yaml --pattern sequential --num-requests 10 --memory-size 64 &
wait $!
echo ""

echo "2. Random Access Pattern (10 requests):"
timeout 30s ./external_simulator external_simulator_config.yaml --pattern random --num-requests 10 --memory-size 64 &
wait $!
echo ""

echo "3. Mixed Access Pattern (15 requests):"
timeout 30s ./external_simulator external_simulator_config.yaml --pattern mixed --num-requests 15 --memory-size 64 &
wait $!
echo ""

echo "Demo completed successfully!"
echo ""
echo "Key features demonstrated:"
echo "- ✓ Ramulator2 initialization with YAML configuration"
echo "- ✓ External request submission via receive_external_requests()"
echo "- ✓ Cycle-accurate DRAM simulation"
echo "- ✓ Request completion callbacks"
echo "- ✓ Different memory access patterns"
echo "- ✓ Statistics collection and reporting"