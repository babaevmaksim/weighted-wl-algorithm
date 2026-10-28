#include "weighted_wl/graph.hpp"
#include "weighted_wl/wl_solver.hpp"
#include <iostream>

using namespace weighted_wl;

int main() {
    std::cout << "[Weighted-WL] Loading financial network benchmark...\n";
    Graph g = load_from_custom_file("tests/financial_network_pro.txt");
    if (g.num_vertices() == 0) {
        g = load_from_custom_file("../tests/financial_network_pro.txt");
    }

    if (g.num_vertices() == 0) {
        std::cerr << "Error: could not open tests/financial_network_pro.txt\n";
        return 1;
    }

    WLSolver solver;
    WLResult res = solver.run(g);

    std::cout << "Converged: " << (res.converged ? "YES" : "NO") << "\n";
    std::cout << "Iterations: " << res.iterations << "\n";
    std::cout << "Classes identified: " << res.num_classes << " / " << g.num_vertices() << "\n";
    std::cout << "Execution time: " << res.elapsed_ms << " ms\n";

    std::cout << "Final colors: ";
    for (size_t v = 0; v < g.num_vertices(); ++v) {
        std::cout << g.colors[v] << " ";
    }
    std::cout << "\n";

    return 0;
}
