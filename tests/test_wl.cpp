#include "weighted_wl/graph.hpp"
#include "weighted_wl/wl_solver.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

using namespace weighted_wl;

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "  [FAIL] " << msg << " (" << #cond << " at " << __FILE__ << ":" << __LINE__ << ")\n"; \
            tests_failed++; \
            return; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        std::cout << "[TEST] Running " << #fn << "...\n"; \
        int prev_failed = tests_failed; \
        fn(); \
        if (tests_failed == prev_failed) { \
            std::cout << "  [PASS] " << #fn << "\n"; \
            tests_passed++; \
        } \
    } while (0)

// ---------------------------------------------------------------------------
// Test 1: Directed Cycle Invariance & Asymmetry Differentiation
// ---------------------------------------------------------------------------
void test_directed_cycles() {
    WLSolver solver;

    // Directed cycle C4: 0 -> 1 -> 2 -> 3 -> 0
    Graph c4_sym(4);
    c4_sym.add_directed_edge(0, 1, 1.0);
    c4_sym.add_directed_edge(1, 2, 1.0);
    c4_sym.add_directed_edge(2, 3, 1.0);
    c4_sym.add_directed_edge(3, 0, 1.0);

    WLResult res_sym = solver.run(c4_sym);
    TEST_ASSERT(res_sym.num_classes == 1, "Symmetric directed cycle C4 must have exactly 1 class");

    // Asymmetric cycle: 0 -> 1 -> 2 <- 3, 0 -> 3 (node 2 is sink with in-deg 2, node 0 is source with out-deg 2)
    Graph c4_asym(4);
    c4_asym.add_directed_edge(0, 1, 1.0);
    c4_asym.add_directed_edge(1, 2, 1.0);
    c4_asym.add_directed_edge(3, 2, 1.0);
    c4_asym.add_directed_edge(0, 3, 1.0);

    WLResult res_asym = solver.run(c4_asym);
    TEST_ASSERT(res_asym.num_classes > 1, "Asymmetric directed graph must be split into multiple classes");
    TEST_ASSERT(c4_asym.colors[0] != c4_asym.colors[2], "Source node 0 and sink node 2 must receive different colors");
}

// ---------------------------------------------------------------------------
// Test 2: Weighted Edge Differentiation
// ---------------------------------------------------------------------------
void test_weighted_edges() {
    WLSolver solver;

    // C4 cycle with 1 heavy edge: (0->1: 5.0, others 1.0)
    Graph g(4);
    g.add_directed_edge(0, 1, 5.0);
    g.add_directed_edge(1, 2, 1.0);
    g.add_directed_edge(2, 3, 1.0);
    g.add_directed_edge(3, 0, 1.0);

    WLResult res = solver.run(g);
    TEST_ASSERT(res.num_classes > 1, "Weighted edge must break symmetry and produce multiple classes");
}

// ---------------------------------------------------------------------------
// Test 3: Numerical Precision & Epsilon Sensitivity Stress Test
// ---------------------------------------------------------------------------
void test_numerical_sensitivity() {
    // 2 isolated vertices with slightly different initial attributes: 1.0 and 1.0 + 1e-12
    Graph g1(2);
    g1.set_node_attribute(0, 1.0);
    g1.set_node_attribute(1, 1.0 + 1e-12);

    // With fine epsilon (1e-14), delta 1e-12 is resolved -> 2 classes
    WLSolverConfig cfg_fine;
    cfg_fine.epsilon = 1e-14;
    WLSolver solver_fine(cfg_fine);
    WLResult res_fine = solver_fine.run(g1);
    TEST_ASSERT(res_fine.num_classes == 2, "Fine epsilon (1e-14) must resolve delta 1e-12 as distinct classes");

    // With coarse epsilon (1e-10), delta 1e-12 is quantized to same bucket -> 1 class
    Graph g2(2);
    g2.set_node_attribute(0, 1.0);
    g2.set_node_attribute(1, 1.0 + 1e-12);

    WLSolverConfig cfg_coarse;
    cfg_coarse.epsilon = 1e-10;
    WLSolver solver_coarse(cfg_coarse);
    WLResult res_coarse = solver_coarse.run(g2);
    TEST_ASSERT(res_coarse.num_classes == 1, "Coarse epsilon (1e-10) must collapse delta 1e-12 into a single class");
}

static Graph safe_load(const std::string& path) {
    Graph g = load_from_custom_file(path);
    if (g.num_vertices() == 0) {
        g = load_from_custom_file("../" + path);
    }
    return g;
}

// ---------------------------------------------------------------------------
// Test 4: Financial Network Benchmark (tests/financial_network_pro.txt)
// ---------------------------------------------------------------------------
void test_financial_network() {
    Graph g = safe_load("tests/financial_network_pro.txt");
    TEST_ASSERT(g.num_vertices() == 12, "Financial network must have 12 vertices");
    TEST_ASSERT(g.num_edges() == 14, "Financial network must have 14 edges");

    WLSolverConfig cfg;
    cfg.epsilon = 1e-14;
    WLSolver solver(cfg);
    WLResult res = solver.run(g);

    TEST_ASSERT(res.num_classes == 11, "Financial network must produce exactly 11 equivalence classes");
    TEST_ASSERT(g.colors[5] == g.colors[6], "Vertices 5 and 6 must have identical colors due to structural symmetry");
    TEST_ASSERT(g.colors[0] != g.colors[1], "Vertices 0 and 1 must have different colors (different balances)");
}

// ---------------------------------------------------------------------------
// Test 5: MUTAG Graph Molecule Symmetry (tests/mutag_1.txt)
// ---------------------------------------------------------------------------
void test_mutag_molecule() {
    Graph g = safe_load("tests/mutag_1.txt");
    TEST_ASSERT(g.num_vertices() == 17, "MUTAG graph 1 must have 17 vertices");

    WLSolverConfig cfg;
    cfg.epsilon = 1e-14;
    WLSolver solver(cfg);
    WLResult res = solver.run(g);

    TEST_ASSERT(res.num_classes == 15, "MUTAG graph 1 must produce exactly 15 classes");
    // Verify physical chemical symmetry pairs: {15, 13} and {16, 12}
    TEST_ASSERT(g.colors[15] == g.colors[13], "Atoms 15 and 13 must share color in MUTAG molecule");
    TEST_ASSERT(g.colors[16] == g.colors[12], "Atoms 16 and 12 must share color in MUTAG molecule");
}

// ---------------------------------------------------------------------------
// Test 6: Isomorphism Detection & Permutation Invariance
// ---------------------------------------------------------------------------
void test_isomorphism_detection() {
    WLSolver solver;

    // Graph G1: 0 -> 1 (w=2.5), 1 -> 2 (w=3.5), 2 -> 0 (w=1.0)
    Graph g1(3);
    g1.set_node_attribute(0, 10.0);
    g1.set_node_attribute(1, 20.0);
    g1.set_node_attribute(2, 30.0);
    g1.add_directed_edge(0, 1, 2.5);
    g1.add_directed_edge(1, 2, 3.5);
    g1.add_directed_edge(2, 0, 1.0);

    // Graph G2: Isomorphic permutation (0->2, 1->0, 2->1)
    Graph g2(3);
    g2.set_node_attribute(2, 10.0);
    g2.set_node_attribute(0, 20.0);
    g2.set_node_attribute(1, 30.0);
    g2.add_directed_edge(2, 0, 2.5);
    g2.add_directed_edge(0, 1, 3.5);
    g2.add_directed_edge(1, 2, 1.0);

    TEST_ASSERT(solver.are_isomorphic(g1, g2), "Permuted isomorphic graphs must be recognized as isomorphic");

    // Graph G3: Perturbed weight on one edge
    Graph g3 = g2;
    g3.adj[2][0].weight = 99.9; // Break isomorphism
    TEST_ASSERT(!solver.are_isomorphic(g1, g3), "Perturbed weight must break isomorphism");
}

int main() {
    std::cout << "======================================================================\n";
    std::cout << "  Weighted Weisfeiler-Lehman (1-WL) Unit Test Suite\n";
    std::cout << "======================================================================\n";

    RUN_TEST(test_directed_cycles);
    RUN_TEST(test_weighted_edges);
    RUN_TEST(test_numerical_sensitivity);
    RUN_TEST(test_financial_network);
    RUN_TEST(test_mutag_molecule);
    RUN_TEST(test_isomorphism_detection);

    std::cout << "======================================================================\n";
    std::cout << "Summary: " << tests_passed << " passed, " << tests_failed << " failed.\n";
    std::cout << "======================================================================\n";

    return (tests_failed == 0) ? 0 : 1;
}
