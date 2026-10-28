#include "weighted_wl/graph.hpp"
#include "weighted_wl/wl_solver.hpp"
#include "weighted_wl/tudataset_reader.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace weighted_wl;

void print_help(const char* prog_name) {
    std::cout << "======================================================================\n"
              << "  Weighted Weisfeiler-Lehman (1-WL) Graph Color Refinement Engine\n"
              << "  Author: Maxim Babaev (SPbU / SPbU Systems Programming Lab)\n"
              << "======================================================================\n\n"
              << "Usage: " << prog_name << " [options]\n\n"
              << "Options:\n"
              << "  -i, --input <file>        Path to input graph file (default: tests/financial_network_pro.txt)\n"
              << "  -f, --format <format>     Format of input file: 'custom' (default) or 'tudataset'\n"
              << "  -d, --dataset <name>      Dataset name if format is 'tudataset' (e.g. MUTAG, ENZYMES)\n"
              << "  -e, --epsilon <val>       Epsilon grid quantization threshold (default: 1e-14)\n"
              << "  -m, --max-iter <N>        Maximum refinement iterations (default: 100)\n"
              << "  -v, --verbose             Enable detailed logging per iteration\n"
              << "  -o, --export-dot <file>   Export final color partition to Graphviz DOT file\n"
              << "  -h, --help                Show this help message and exit\n\n"
              << "Examples:\n"
              << "  " << prog_name << " -i tests/financial_network_pro.txt -v\n"
              << "  " << prog_name << " -i tests/mutag_1.txt -e 1e-14 -o mutag.dot\n"
              << "  " << prog_name << " -f tudataset -i data/MUTAG -d MUTAG\n";
}

int main(int argc, char* argv[]) {
    std::string input_path = "tests/financial_network_pro.txt";
    std::string format = "custom";
    std::string dataset_name = "MUTAG";
    std::string export_dot_path = "";
    double epsilon = 1e-14;
    size_t max_iterations = 100;
    bool verbose = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            input_path = argv[++i];
        } else if ((arg == "-f" || arg == "--format") && i + 1 < argc) {
            format = argv[++i];
        } else if ((arg == "-d" || arg == "--dataset") && i + 1 < argc) {
            dataset_name = argv[++i];
        } else if ((arg == "-e" || arg == "--epsilon") && i + 1 < argc) {
            epsilon = std::stod(argv[++i]);
        } else if ((arg == "-m" || arg == "--max-iter") && i + 1 < argc) {
            max_iterations = static_cast<size_t>(std::stoi(argv[++i]));
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if ((arg == "-o" || arg == "--export-dot") && i + 1 < argc) {
            export_dot_path = argv[++i];
        } else {
            std::cerr << "[Warning] Unknown argument: " << arg << " (run with --help for usage)\n";
        }
    }

    std::cout << "======================================================================\n"
              << "  Weighted Weisfeiler-Lehman (1-WL) Algorithm Execution\n"
              << "======================================================================\n";
    std::cout << "[Config] Format:     " << format << "\n"
              << "[Config] Input:      " << input_path << "\n"
              << "[Config] Epsilon:    " << std::scientific << epsilon << std::defaultfloat << "\n"
              << "[Config] Max Iter:   " << max_iterations << "\n\n";

    WLSolverConfig cfg;
    cfg.epsilon = epsilon;
    cfg.max_iterations = max_iterations;
    cfg.verbose = verbose;
    WLSolver solver(cfg);

    if (format == "tudataset") {
        std::cout << "[Loading] Parsing TUDataset collection from: " << input_path << " (dataset: " << dataset_name << ")...\n";
        std::vector<Graph> dataset = TUDatasetReader::load_dataset(input_path, dataset_name);
        if (dataset.empty()) {
            std::cerr << "[Error] Failed to load graphs from TUDataset.\n";
            return 1;
        }

        std::cout << "[Loaded] " << dataset.size() << " graphs parsed successfully.\n";
        double total_ms = 0.0;
        size_t total_nodes = 0;
        size_t total_edges = 0;

        for (size_t g_idx = 0; g_idx < dataset.size(); ++g_idx) {
            total_nodes += dataset[g_idx].num_vertices();
            total_edges += dataset[g_idx].num_edges();
            WLResult res = solver.run(dataset[g_idx]);
            total_ms += res.elapsed_ms;
        }

        std::cout << "\n======================================================================\n"
                  << "  TUDataset Processing Summary (" << dataset_name << ")\n"
                  << "======================================================================\n"
                  << "Total Graphs:       " << dataset.size() << "\n"
                  << "Total Vertices:     " << total_nodes << "\n"
                  << "Total Edges:        " << total_edges << "\n"
                  << "Total Time:         " << total_ms << " ms (" << std::fixed << std::setprecision(4) << total_ms / 1000.0 << " s)\n"
                  << "Average per graph:  " << std::setprecision(3) << total_ms / dataset.size() << " ms\n";
        return 0;
    }

    // Default: Custom single graph file
    Graph g = load_from_custom_file(input_path);
    if (g.num_vertices() == 0) {
        std::cerr << "[Error] Graph contains 0 vertices or failed to load from: " << input_path << std::endl;
        return 1;
    }

    std::cout << "[Graph] Vertices: " << g.num_vertices() << ", Edges: " << g.num_edges() << "\n";
    std::cout << "[Run] Executing Weighted 1-WL refinement...\n\n";

    WLResult result = solver.run(g);

    std::cout << "\n======================================================================\n"
              << "  Refinement Results\n"
              << "======================================================================\n";
    std::cout << "Stabilized:         " << (result.converged ? "YES" : "NO (max iterations reached)") << "\n";
    std::cout << "Iterations:         " << result.iterations << "\n";
    std::cout << "Classes Identified: " << result.num_classes << " / " << g.num_vertices() << "\n";
    std::cout << "Elapsed Time:       " << std::fixed << std::setprecision(4) << result.elapsed_ms << " ms\n";

    std::cout << "\nClass Partition Evolution:\n";
    for (size_t it = 0; it < result.class_counts_per_iteration.size(); ++it) {
        std::cout << "  Step " << it << ": " << result.class_counts_per_iteration[it] << " unique classes\n";
    }

    std::cout << "\nFinal Color Assignments per Vertex:\n  ";
    for (size_t v = 0; v < g.num_vertices(); ++v) {
        std::cout << "v" << v << "=" << g.colors[v] << " ";
        if ((v + 1) % 8 == 0 && v + 1 < g.num_vertices()) {
            std::cout << "\n  ";
        }
    }
    std::cout << "\n";

    if (!export_dot_path.empty()) {
        if (export_to_dot(g, export_dot_path)) {
            std::cout << "\n[Export] Graphviz DOT exported to: " << export_dot_path << "\n";
        } else {
            std::cerr << "\n[Error] Failed to write DOT file to: " << export_dot_path << "\n";
        }
    }

    return 0;
}

