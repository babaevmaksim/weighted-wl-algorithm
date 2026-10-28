# Weighted Weisfeiler-Lehman (1-WL) Algorithm

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.14+-064F8C.svg?logo=cmake)](https://cmake.org/)
[![Tests](https://img.shields.io/badge/tests-6%20passed%2C%20100%25-brightgreen.svg)]()
[![Docker](https://img.shields.io/badge/docker-ready-blue.svg?logo=docker)]()

High-performance, numerically stable C++17 library and command-line engine for **Weisfeiler-Lehman (1-WL) graph color refinement** on **directed graphs with continuous vertex attributes and real-valued edge weights**.

---

## Table of Contents

- [Overview & Motivation](#overview--motivation)
- [Relevance to Systems Programming](#relevance-to-systems-programming)
- [Mathematical Model](#mathematical-model)
  - [Lexicographical Neighbor Tuples](#lexicographical-neighbor-tuples)
  - [Grid Quantization for IEEE 754 Transitivity](#grid-quantization-for-ieee-754-transitivity)
  - [Complexity Analysis](#complexity-analysis)
- [Repository Structure](#repository-structure)
- [Quick Start & Build](#quick-start--build)
  - [Prerequisites](#prerequisites)
  - [Building with CMake](#building-with-cmake)
  - [Running Unit Tests](#running-unit-tests)
  - [Running Benchmarks](#running-benchmarks)
  - [Docker Execution](#docker-execution)
- [Command-Line Interface (CLI)](#command-line-interface-cli)
- [C++ Library API Example](#c-library-api-example)
- [Experimental Results](#experimental-results)
  - [1. Resolving Power Comparison](#1-resolving-power-comparison)
  - [2. Scalability on Large Synthetic Graphs](#2-scalability-on-large-synthetic-graphs)
  - [3. TUDataset Benchmark (MUTAG)](#3-tudataset-benchmark-mutag)
- [Citation & Academic Context](#citation--academic-context)
- [License](#license)

---

## Overview & Motivation

The classical **1-Weisfeiler-Lehman (1-WL)** heuristic (Weisfeiler & Leman, 1968) is the foundational cornerstone of algorithmic graph isomorphism testing, graph kernels (Shervashidze et al., 2011), and message-passing Graph Neural Networks (Xu et al., 2018; Morris et al., 2019).

However, classical 1-WL exhibits two critical limitations:
1. **Topological Symmetry Bias:** It assumes undirected edges, completely discarding directional asymmetry (in-degrees vs. out-degrees).
2. **Discrete Label Restriction:** It operates strictly on discrete categorical labels. Real-world applications rely on continuous physical features (node attributes) and real-valued link weights (latencies, bandwidth, transaction volumes).

This repository provides an optimized, mathematically sound extension of 1-WL supporting:
* **Directed edges** with explicit incoming (`-1`) and outgoing (`+1`) message-passing channels.
* **Continuous real-valued edge weights** and node attributes.
* **Provably transitive numerical stability** under IEEE 754 floating-point arithmetic.
* **Sub-millisecond performance** on graphs with tens of thousands of edges.

---

## Relevance to Systems Programming

While graph isomorphism is often treated as abstract combinatorics, directed weighted 1-WL directly addresses key challenges in **systems programming and infrastructure engineering**:

* **Storage Systems & Distributed I/O (SAN/NAS):** Modeling transactional dependency graphs between asynchronous storage controllers. Vertices represent I/O requests or cache blocks; directed edges represent read/write causal dependencies; weights correspond to queue latency or transfer sizes.
* **Compiler Analysis & Optimization (RISC-V / LLVM):** Control Flow Graphs (CFGs) and Instruction Dependency Graphs are directed and weighted (execution frequencies from profiling, pipeline latency). Subgraph canonicalization detects recurring basic-block idioms, enables loop unrolling/vectorization, and optimizes register allocation.
* **Numerical Determinism in System Software:** Comparing continuous metrics across distributed nodes often causes non-deterministic bugs due to IEEE 754 precision issues. Our grid quantization mechanism guarantees a mathematically rigorous equivalence relation with zero floating-point drift.

---

## Mathematical Model

Let $G = (V, E, A, W)$ be a directed, attributed, weighted graph:
* $V$ is the set of vertices ($|V| = n$).
* $E \subseteq V \times V$ is the set of directed edges ($|E| = m$).
* $A: V \to \mathbb{R}$ is the node attribute function.
* $W: E \to \mathbb{R}$ is the edge weight function.

### Lexicographical Neighbor Tuples

On iteration $t+1$, the relationship between vertex $v$ and adjacent vertex $u \in N(v)$ is captured by the state tuple:
$$S_{uv}^{(t)} = \left(\text{type}_{uv}, C_u^{(t)}, \text{disc}(w_{uv})\right)$$
where:
* $\text{type}_{uv} \in \{+1, -1\}$ distinguishes outgoing ($+1$) from incoming ($-1$) links.
* $C_u^{(t)} \in \mathbb{N}$ is the canonical discrete color identifier of neighbor $u$.
* $\text{disc}(w_{uv}) = \text{round}(w_{uv} / \varepsilon) \cdot \varepsilon$ is the discretized weight.

The color refinement update rule is:
$$C_v^{(t+1)} = \text{RECOLOR}\left(C_v^{(t)}, \text{SORT}\left(\bigcup_{u \in N(v)} \{S_{uv}^{(t)}\}\right)\right)$$

Standard `std::tuple` implements strict weak ordering left-to-right:
1. **Topological direction** (`type`) has highest priority.
2. **Neighbor color** ($C_u$) has second priority.
3. **Link weight** ($w_{uv}$) has third priority.

### Grid Quantization for IEEE 754 Transitivity

A naive $\varepsilon$-neighborhood relation $|w_1 - w_2| < \varepsilon$ is **not transitive** ($A \approx B \land B \approx C \centernot\implies A \approx C$). Using non-transitive predicates inside `std::map` or `std::sort` leads to undefined behavior and cycle faults.

To guarantee mathematical consistency, we apply uniform grid quantization:
$$\text{disc}(x, \varepsilon) = \text{round}\left(\frac{x}{\varepsilon}\right) \cdot \varepsilon$$
This partitions the real line into discrete buckets $[k\varepsilon - \varepsilon/2, k\varepsilon + \varepsilon/2)$, strictly preserving reflexivity, symmetry, and transitivity.

### Complexity Analysis

For a graph with $n$ vertices and $m$ directed edges:
1. **Message collection:** $O(m)$ steps.
2. **Lexicographical sorting:** $\sum_v O(d_v \log d_v) \le O(m \log d_{\max})$ where $d_{\max}$ is maximum vertex degree.
3. **Canonical color assignment:** $O(m \log K)$ where $K \le n$ is the number of unique partition classes.

**Total per-iteration complexity:** $\mathcal{O}(m \log d_{\max} + m \log K)$ — virtually linear in the number of edges.

---

## Repository Structure

```
weighted-wl-algorithm/
├── CMakeLists.txt            # Modern target-based CMake build configuration
├── Dockerfile                # Multi-stage Docker build for isolated reproduction
├── .clang-format             # LLVM/Google C++ code formatting standard
├── .gitignore                # Git ignore rules for build artifacts
├── LICENSE                   # MIT Open Source License
├── README.md                 # Project documentation
│
├── include/weighted_wl/      # Public C++ API headers
│   ├── graph.hpp             # Directed weighted graph data structure
│   ├── wl_solver.hpp         # Weighted 1-WL engine and isomorphism checker
│   └── tudataset_reader.hpp  # Standard TUDataset benchmark loader
│
├── src/                      # Library implementation
│   ├── graph.cpp             # Graph operations and Graphviz DOT export
│   ├── wl_solver.cpp         # Refinement loop and canonical hashing
│   ├── tudataset_reader.cpp  # TUDataset parser
│   └── graph.h               # Legacy backward-compatibility wrapper
│
├── app/                      # Standalone CLI application
│   └── main.cpp              # Full-featured command-line tool
│
├── tests/                    # Automated unit tests
│   ├── test_wl.cpp           # Test suite (cycles, weights, precision, isomorphism)
│   ├── financial_network_pro.txt # 12-node benchmark network
│   └── mutag_1.txt           # 17-node molecular graph
│
├── benchmarks/               # Performance and scalability evaluation
│   └── benchmark.cpp         # Micro- and macro-benchmarks (TUDataset, synthetic)
│
├── data/MUTAG/               # Included TUDataset benchmark (188 graphs)
│   ├── MUTAG_A.txt
│   ├── MUTAG_graph_indicator.txt
│   ├── MUTAG_node_labels.txt
│   └── MUTAG_edge_labels.txt
│
├── scripts/                  # Automated execution scripts
│   ├── run_experiments.py   # Cross-platform Python reproduction script
│   └── run_experiments.sh   # Bash reproduction script
│
└── .github/workflows/        # Continuous Integration
    └── ci.yml                # GitHub Actions workflow (Ubuntu & Windows)
```

---

## Quick Start & Build

### Prerequisites
* C++17 compatible compiler (`g++` >= 9.0, `clang++` >= 10.0, or MSVC >= 2019)
* CMake >= 3.14
* Build generator: `Ninja` or `make`

### Building with CMake

```bash
# Clone the repository
git clone https://github.com/babaevmaksim/weighted-wl-algorithm.git
cd weighted-wl-algorithm

# Configure and compile (Release mode)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Running Unit Tests

Run the built-in test suite:
```bash
./build/wl_tests
# Alternatively via CTest:
cd build && ctest --output-on-failure
```

Output:
```
======================================================================
  Weighted Weisfeiler-Lehman (1-WL) Unit Test Suite
======================================================================
[TEST] Running test_directed_cycles...       [PASS]
[TEST] Running test_weighted_edges...        [PASS]
[TEST] Running test_numerical_sensitivity... [PASS]
[TEST] Running test_financial_network...     [PASS]
[TEST] Running test_mutag_molecule...        [PASS]
[TEST] Running test_isomorphism_detection... [PASS]
======================================================================
Summary: 6 passed, 0 failed.
======================================================================
```

### Running Benchmarks

```bash
./build/wl_benchmark
```

### Docker Execution

To build and run in an isolated Linux container:
```bash
docker build -t weighted-wl .
docker run --rm weighted-wl
```

---

## Command-Line Interface (CLI)

The `wl_cli` tool provides a flexible command-line interface:

```bash
./build/wl_cli [options]
```

### Supported Flags

| Option | Argument | Description | Default |
|---|---|---|---|
| `-i`, `--input` | `<path>` | Path to input graph file | `tests/financial_network_pro.txt` |
| `-f`, `--format` | `custom\|tudataset` | Format of input dataset | `custom` |
| `-d`, `--dataset` | `<name>` | Dataset prefix for TUDataset | `MUTAG` |
| `-e`, `--epsilon` | `<val>` | Grid quantization threshold $\varepsilon$ | `1e-14` |
| `-m`, `--max-iter` | `<N>` | Maximum refinement iterations | `100` |
| `-v`, `--verbose` | — | Print per-iteration progress | `false` |
| `-o`, `--export-dot` | `<path>` | Export final partition to Graphviz DOT | none |
| `-h`, `--help` | — | Display usage information | — |

### Example CLI Invocations

1. **Detailed refinement with Graphviz visualization:**
   ```bash
   ./build/wl_cli -i tests/financial_network_pro.txt -v -o financial.dot
   dot -Tpng financial.dot -o financial.png
   ```

2. **Batch evaluation on full TUDataset MUTAG:**
   ```bash
   ./build/wl_cli -f tudataset -i data/MUTAG -d MUTAG
   ```

---

## C++ Library API Example

```cpp
#include "weighted_wl/graph.hpp"
#include "weighted_wl/wl_solver.hpp"
#include <iostream>

int main() {
    using namespace weighted_wl;

    // 1. Construct a directed weighted graph
    Graph g(4);
    g.set_node_attribute(0, 100.0);
    g.set_node_attribute(1, 200.0);
    g.set_node_attribute(2, 300.0);
    g.set_node_attribute(3, 400.0);

    g.add_directed_edge(0, 1, 10.5);
    g.add_directed_edge(1, 2, 20.0);
    g.add_directed_edge(2, 3, 5.0);
    g.add_directed_edge(3, 0, 1.0);

    // 2. Configure and run the solver
    WLSolverConfig config;
    config.epsilon = 1e-14;
    config.verbose = true;

    WLSolver solver(config);
    WLResult result = solver.run(g);

    std::cout << "Converged: " << (result.converged ? "YES" : "NO") << "\n";
    std::cout << "Iterations: " << result.iterations << "\n";
    std::cout << "Classes: " << result.num_classes << "\n";
    std::cout << "Time: " << result.elapsed_ms << " ms\n";

    return 0;
}
```

---

## Experimental Results

### 1. Resolving Power Comparison

| Graph Configuration | Vertices / Edges | Classical 1-WL | Weighted 1-WL (Ours) | Note |
|---|:---:|:---:|:---:|---|
| Symmetric Cycle $C_4$ | 4 / 4 | 1 class | **1 class** | Correctly preserves symmetry |
| $C_4$ with Asymmetric Weights | 4 / 4 | 1 class (failure) | **2 classes** | Distinguishes endpoints of heavy link |
| Financial Transaction Network | 12 / 14 | 3 classes | **11 classes** | Correctly isolates nodes 5 and 6 as sole symmetric pair |
| Molecular Graph (MUTAG-1) | 17 / 38 | 3 classes | **15 classes** | Accurately identifies physical chemical symmetry |

### 2. Scalability on Large Synthetic Graphs

Benchmarked on an Intel Core i7 @ 2.7 GHz (single thread, `-O3`):

| Vertices ($n$) | Edges ($m$) | Unique Classes | Iterations to Stabilize | Execution Time |
|:---:|:---:|:---:|:---:|:---:|
| 100 | 500 | 100 | 1 | **0.18 ms** |
| 500 | 2,500 | 500 | 1 | **0.76 ms** |
| 1,000 | 5,000 | 1,000 | 1 | **1.53 ms** |
| 5,000 | 25,000 | 5,000 | 1 | **8.15 ms** |
| 10,000 | 50,000 | 10,000 | 1 | **18.19 ms** |

### 3. TUDataset Benchmark (MUTAG)

Processing the entire MUTAG collection (188 molecular graphs, 3,371 atoms, 7,442 bonds):
* **Total computation time:** **5.67 ms**
* **Average latency per graph:** **0.030 ms** (30 microseconds)

---

## Citation & Academic Context

This project was developed as part of academic research at **Saint Petersburg State University (SPbU)**, Faculty of Mathematics and Computer Science, Department of Systems Programming.

* **Author:** Maxim Babaev (SPbU)
* **Scientific Advisor:** Yu. A. Andreev (Senior Lecturer, Department of Systems Programming, SPbU)

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.
