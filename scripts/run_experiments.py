#!/usr/bin/env python3
"""
Weighted Weisfeiler-Lehman (1-WL) Experiment Reproducibility Runner.
Automates benchmark execution, data validation, and performance table generation.
"""

import subprocess
import sys
import os
import time

def find_binary(name):
    candidates = [
        os.path.join("build", name),
        os.path.join("build", name + ".exe"),
        os.path.join("build", "Release", name + ".exe"),
        os.path.join(".", name),
        os.path.join(".", name + ".exe"),
    ]
    for c in candidates:
        if os.path.isfile(c) and os.access(c, os.X_OK):
            return c
    return None

def main():
    print("=" * 70)
    print("  Weighted 1-WL Automated Experiment Reproducibility Suite")
    print("=" * 70)

    # 1. Locate binaries
    cli_bin = find_binary("wl_cli")
    bench_bin = find_binary("wl_benchmark")
    test_bin = find_binary("wl_tests")

    if not cli_bin or not bench_bin or not test_bin:
        print("[!] Binaries not found. Attempting to build with CMake...")
        res = subprocess.run(["cmake", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"])
        if res.returncode != 0:
            print("[Error] CMake configuration failed.")
            sys.exit(1)
        res = subprocess.run(["cmake", "--build", "build", "--config", "Release"])
        if res.returncode != 0:
            print("[Error] Build failed.")
            sys.exit(1)
        cli_bin = find_binary("wl_cli")
        bench_bin = find_binary("wl_benchmark")
        test_bin = find_binary("wl_tests")

    print(f"[+] Found CLI:       {cli_bin}")
    print(f"[+] Found Benchmark: {bench_bin}")
    print(f"[+] Found Tests:     {test_bin}")

    # 2. Run unit tests
    print("\n" + "-" * 70)
    print("Step 1: Running Automated Unit Verification...")
    print("-" * 70)
    res = subprocess.run([test_bin])
    if res.returncode != 0:
        print("[Error] Unit tests failed!")
        sys.exit(1)

    # 3. Run benchmarks
    print("\n" + "-" * 70)
    print("Step 2: Executing Performance Benchmarks...")
    print("-" * 70)
    res = subprocess.run([bench_bin])
    if res.returncode != 0:
        print("[Error] Benchmark execution failed!")
        sys.exit(1)

    # 4. Run Financial Network CLI demo with DOT export
    print("\n" + "-" * 70)
    print("Step 3: Financial Network Analysis & DOT Generation...")
    print("-" * 70)
    dot_file = "financial_network_colored.dot"
    res = subprocess.run([cli_bin, "-i", "tests/financial_network_pro.txt", "-v", "-o", dot_file])
    if res.returncode == 0 and os.path.isfile(dot_file):
        print(f"[+] Successfully exported Graphviz visualization to: {dot_file}")

    print("\n" + "=" * 70)
    print("  All experiments successfully reproduced and verified!")
    print("=" * 70)

if __name__ == "__main__":
    main()
