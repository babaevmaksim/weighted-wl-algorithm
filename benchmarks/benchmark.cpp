#include "weighted_wl/graph.hpp"
#include "weighted_wl/wl_solver.hpp"
#include "weighted_wl/tudataset_reader.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

using namespace weighted_wl;

Graph generate_synthetic_graph(size_t num_vertices, size_t num_edges, unsigned seed = 42) {
    Graph g(num_vertices);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> attr_dist(1.0, 100.0);
    std::uniform_real_distribution<double> weight_dist(0.1, 50.0);
    std::uniform_int_distribution<int> node_dist(0, static_cast<int>(num_vertices - 1));

    for (size_t v = 0; v < num_vertices; ++v) {
        g.set_node_attribute(v, attr_dist(rng));
    }

    for (size_t e = 0; e < num_edges; ++e) {
        int u = node_dist(rng);
        int v = node_dist(rng);
        if (u != v) {
            g.add_directed_edge(u, v, weight_dist(rng));
        }
    }

    return g;
}

int main() {
    std::cout << "======================================================================\n"
              << "  Weighted 1-WL Scalability & Performance Benchmarks\n"
              << "======================================================================\n\n";

    WLSolver solver;

    // 1. Financial Network
    std::cout << "[1] Financial Network (tests/financial_network_pro.txt)...\n";
    Graph fin_g = load_from_custom_file("tests/financial_network_pro.txt");
    if (fin_g.num_vertices() > 0) {
        WLResult res = solver.run(fin_g);
        std::cout << "    Vertices: " << fin_g.num_vertices() << ", Edges: " << fin_g.num_edges()
                  << " -> " << res.num_classes << " classes in " << res.iterations << " iters ("
                  << std::fixed << std::setprecision(4) << res.elapsed_ms << " ms)\n";
    }

    // 2. MUTAG 1 Molecule
    std::cout << "[2] MUTAG Single Molecule (tests/mutag_1.txt)...\n";
    Graph mut_g = load_from_custom_file("tests/mutag_1.txt");
    if (mut_g.num_vertices() > 0) {
        WLResult res = solver.run(mut_g);
        std::cout << "    Vertices: " << mut_g.num_vertices() << ", Edges: " << mut_g.num_edges()
                  << " -> " << res.num_classes << " classes in " << res.iterations << " iters ("
                  << std::fixed << std::setprecision(4) << res.elapsed_ms << " ms)\n";
    }

    // 3. Synthetic Scalability Suite
    std::cout << "\n[3] Synthetic Scalability Evaluation (Testing O(m log d_max + m log K)):\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(15) << "Vertices"
              << std::setw(15) << "Edges"
              << std::setw(12) << "Classes"
              << std::setw(12) << "Iters"
              << std::setw(15) << "Time (ms)" << "\n";
    std::cout << "----------------------------------------------------------------------\n";

    std::vector<std::pair<size_t, size_t>> test_sizes = {
        {100, 500},
        {500, 2500},
        {1000, 5000},
        {5000, 25000},
        {10000, 50000}
    };

    for (auto [N, M] : test_sizes) {
        Graph g = generate_synthetic_graph(N, M);
        WLResult res = solver.run(g);
        std::cout << std::left << std::setw(15) << N
                  << std::setw(15) << M
                  << std::setw(12) << res.num_classes
                  << std::setw(12) << res.iterations
                  << std::setw(15) << std::fixed << std::setprecision(3) << res.elapsed_ms << "\n";
    }
    std::cout << "----------------------------------------------------------------------\n\n";

    // 4. Batch TUDataset Evaluation if MUTAG exists
    std::vector<std::string> candidate_paths = {
        "data/MUTAG",
        "../../Учебная практика/файлы/MUTAG",
        "../Учебная практика/файлы/MUTAG"
    };

    std::string found_path = "";
    for (const auto& p : candidate_paths) {
        std::ifstream test_f(p + "/MUTAG_A.txt");
        if (test_f.is_open()) {
            found_path = p;
            break;
        }
    }

    if (!found_path.empty()) {
        std::cout << "[4] Full TUDataset MUTAG Batch Processing (from " << found_path << ")...\n";
        std::vector<Graph> mutag_collection = TUDatasetReader::load_dataset(found_path, "MUTAG");
        if (!mutag_collection.empty()) {
            double batch_ms = 0.0;
            size_t total_v = 0, total_e = 0;
            for (auto& mg : mutag_collection) {
                total_v += mg.num_vertices();
                total_e += mg.num_edges();
                WLResult r = solver.run(mg);
                batch_ms += r.elapsed_ms;
            }
            std::cout << "    Processed " << mutag_collection.size() << " molecular graphs ("
                      << total_v << " total atoms, " << total_e << " total bonds)\n"
                      << "    Total CPU time: " << batch_ms << " ms ("
                      << batch_ms / mutag_collection.size() << " ms / graph)\n";
        }
    }

    std::cout << "\n[Done] Benchmark completed successfully.\n";
    return 0;
}
