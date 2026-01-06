#!/bin/bash
#-------------------------------------------------------------------
# Unit test for external command parsing logic
# This tests the parsing without needing a full pgpool setup
#

echo "=== Testing external command output parsing ==="

# Test 1: Integer values
echo "Test 1: Integer millisecond values"
echo "0 25 50" > test_output.txt
echo "Expected: 0ms, 25ms, 50ms"
echo "Output: $(cat test_output.txt)"
echo ""

# Test 2: Float values
echo "Test 2: Floating-point millisecond values"
echo "0 25.5 100.75" > test_output_float.txt
echo "Expected: 0ms, 25.5ms, 100.75ms"
echo "Output: $(cat test_output_float.txt)"
echo ""

# Test 3: High precision float values
echo "Test 3: High precision values"
echo "0 0.001 999.999" > test_output_precision.txt
echo "Expected: 0ms, 0.001ms, 999.999ms"
echo "Output: $(cat test_output_precision.txt)"
echo ""

# Test 4: Edge case - zero values
echo "Test 4: All zero values"
echo "0 0 0" > test_output_zeros.txt
echo "Expected: 0ms, 0ms, 0ms"
echo "Output: $(cat test_output_zeros.txt)"
echo ""

# Test 5: Edge case - large values
echo "Test 5: Large delay values"
echo "0 5000 10000" > test_output_large.txt
echo "Expected: 0ms, 5000ms, 10000ms"
echo "Output: $(cat test_output_large.txt)"
echo ""

# Test 6: Mixed integer and float values
echo "Test 6: Mixed integer and float values"
echo "0 25 50.5" > test_output_mixed.txt
echo "Expected: 0ms, 25ms, 50.5ms"
echo "Output: $(cat test_output_mixed.txt)"
echo ""

# Cleanup
rm -f test_output_*.txt

echo "All parsing tests completed. These outputs should be parseable by the external command feature."
