#!/usr/bin/env python3
"""
Convert VCD-based memory trace to SimpleO3 instruction trace format with timing.

Input format:  <cycle> <address> <operation> [data]
Output format: <bubble_count> <load_address> [store_address]

SimpleO3 format:
- bubble_count: cycles to wait before this memory operation
- load_address: address for load (use -1 if no load)
- store_address: address for store (optional, only if store operation)
"""

def convert_to_simpleO3_trace(input_file, output_file):
    operations = []

    # Parse input trace
    with open(input_file, 'r') as infile:
        for line_num, line in enumerate(infile, 1):
            line = line.strip()
            # Skip comments and empty lines
            if line.startswith('#') or not line:
                continue

            tokens = line.split()
            if len(tokens) >= 3:
                try:
                    cycle = int(tokens[0])
                    address = tokens[1]
                    operation = tokens[2].upper()

                    # Convert hex addresses to decimal
                    if address.startswith('0x') or address.startswith('0X'):
                        addr_val = int(address, 16)
                    else:
                        addr_val = int(address)

                    operations.append((cycle, addr_val, operation))
                except ValueError as e:
                    print(f"Warning: Skipping malformed line {line_num}: {line} ({e})")
                    continue

    if not operations:
        print("Error: No valid operations found in input file")
        return

    # Sort by cycle time to ensure proper ordering
    operations.sort(key=lambda x: x[0])

    # Convert to SimpleO3 format
    with open(output_file, 'w') as outfile:
        prev_cycle = 0

        for cycle, address, operation in operations:
            # Calculate bubble count (cycles since last operation)
            bubble_count = cycle - prev_cycle
            prev_cycle = cycle

            # Convert to SimpleO3 format
            if operation == 'READ':
                # Format: <bubble_count> <load_address>
                outfile.write(f"{bubble_count} {address}\n")
            elif operation == 'WRITE':
                # Format: <bubble_count> <load_address> <store_address>
                # For write-only, we use the same address for both load and store
                # This is a limitation of SimpleO3 format - it expects instruction-like behavior
                outfile.write(f"{bubble_count} {address} {address}\n")

    print(f"Converted {len(operations)} operations to {output_file}")
    print(f"Total simulation cycles: {operations[-1][0]}")

if __name__ == "__main__":
    convert_to_simpleO3_trace("output.trace", "output_timing_aware.trace")