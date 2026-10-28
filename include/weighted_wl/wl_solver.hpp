#ifndef WEIGHTED_WL_SOLVER_HPP
#define WEIGHTED_WL_SOLVER_HPP

#include "graph.hpp"
#include <vector>
#include <tuple>
#include <map>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace weighted_wl {

/**
 * @brief Configuration parameters for Weighted Weisfeiler-Lehman solver.
 */
struct WLSolverConfig {
    double epsilon = 1e-14;         ///< Numerical sensitivity threshold for IEEE 754 float comparison
    size_t max_iterations = 100;    ///< Safety iteration limit
    bool verbose = false;           ///< Print per-iteration diagnostics to std::cout
};

/**
 * @brief Output result of the WL refinement procedure.
 */
struct WLResult {
    int iterations = 0;
    int num_classes = 0;
    bool converged = false;
    std::vector<uint64_t> final_colors;
    std::vector<int> class_counts_per_iteration;
    double elapsed_ms = 0.0;
};

/**
 * @brief Weighted Weisfeiler-Lehman (1-WL) graph color refinement engine.
 *
 * Extends the classical 1-WL isomorphism test to directed graphs with
 * continuous/discrete edge weights and continuous/discrete node attributes.
 *
 * Key algorithmic properties:
 * 1. Directed topology invariance: outgoing (+1) and incoming (-1) edges are
 *    distinguished in neighbor tuples, preserving orientation semantics.
 * 2. Transitive numerical stability: grid quantization round(w / eps) * eps ensures
 *    that floating-point equivalence is an exact equivalence relation (reflexive,
 *    symmetric, and transitive), preventing invalid std::map ordering.
 * 3. Lexicographical tuple hierarchy: tuples (type, neighbor_color, edge_weight)
 *    are sorted under strict weak ordering, ensuring permutation invariance.
 */
class WLSolver {
public:
    explicit WLSolver(WLSolverConfig config = WLSolverConfig{})
        : config_(config) {}

    /**
     * @brief Discretizes a floating-point value to a grid step of size epsilon.
     * Guaranteed strictly transitive: discretize(a) == discretize(b) iff
     * both round to the exact same grid node.
     */
    static double discretize(double value, double epsilon) {
        if (epsilon <= 0.0) return value;
        return std::round(value / epsilon) * epsilon;
    }

    /**
     * @brief Sets or updates the solver configuration.
     */
    void set_config(const WLSolverConfig& config) {
        config_ = config;
    }

    const WLSolverConfig& config() const {
        return config_;
    }

    /**
     * @brief Count unique partition colors in the graph.
     */
    static int count_unique_colors(const Graph& g);

    /**
     * @brief Initialize vertex colors from continuous initial attributes
     * via grid quantization and canonical color assignment.
     */
    void initialize_colors(Graph& g) const;

    /**
     * @brief Perform a single refinement step on graph g.
     */
    void refine_step(Graph& g) const;

    /**
     * @brief Run color refinement until partition stabilization or max_iterations reached.
     */
    WLResult run(Graph& g) const;

    /**
     * @brief Checks if two graphs are isomorphic according to the Weighted 1-WL test.
     * Returns false if WL proves non-isomorphism (guaranteed sound).
     * If true, graphs are indistinguishable under 1-WL.
     */
    bool are_isomorphic(Graph& g1, Graph& g2) const;

private:
    WLSolverConfig config_;
};

// Legacy C-style interface for compatibility with earlier code and Appendix A
void refine_colors(Graph& g);

} // namespace weighted_wl

#endif // WEIGHTED_WL_SOLVER_HPP

