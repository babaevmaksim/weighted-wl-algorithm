#!/usr/bin/env bash
set -e

echo "======================================================================"
echo "  Weighted 1-WL Experiment Reproduction Script"
echo "======================================================================"

mkdir -p build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

echo ""
echo "Running Unit Tests..."
./build/wl_tests

echo ""
echo "Running Benchmarks..."
./build/wl_benchmark

echo ""
echo "Running Financial Network Demo..."
./build/wl_cli -i tests/financial_network_pro.txt -v -o tests/financial_network.dot

echo ""
echo "Done! All experiments verified."
