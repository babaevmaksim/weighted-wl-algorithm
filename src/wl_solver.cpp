#include "weighted_wl/wl_solver.hpp"
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <iostream>
#include <iomanip>

namespace weighted_wl {

int WLSolver::count_unique_colors(const Graph& g) {
    std::unordered_set<uint64_t> unique_set(g.colors.begin(), g.colors.end());
    return static_cast<int>(unique_set.size());
}

void WLSolver::initialize_colors(Graph& g) const {
    if (g.num_vertices() == 0) return;

    if (g.initial_attributes.empty()) {
        g.colors.assign(g.num_vertices(), 0);
        return;
    }

    // 1. Collect all unique discretized attributes in a sorted set
    std::set<double> sorted_attrs;
    for (size_t v = 0; v < g.num_vertices(); ++v) {
        sorted_attrs.insert(discretize(g.initial_attributes[v], config_.epsilon));
    }

    // 2. Canonical mapping: sorted order guarantees permutation invariance
    std::unordered_map<double, uint64_t> attr_to_id;
    uint64_t id = 0;
    for (double attr : sorted_attrs) {
        attr_to_id[attr] = id++;
    }

    // 3. Assign canonical initial color IDs
    g.colors.resize(g.num_vertices());
    for (size_t v = 0; v < g.num_vertices(); ++v) {
        double disc = discretize(g.initial_attributes[v], config_.epsilon);
        g.colors[v] = attr_to_id[disc];
    }
}

void WLSolver::refine_step(Graph& g) const {
    const size_t n = g.num_vertices();
    if (n == 0) return;

    using NeighborTuple = std::tuple<int, uint64_t, double>;
    using Signature = std::pair<uint64_t, std::vector<NeighborTuple>>;

    std::vector<Signature> vertex_signatures(n);
    std::set<Signature> unique_signatures;

    std::vector<NeighborTuple> neighbors;
    for (size_t v = 0; v < n; ++v) {
        neighbors.clear();
        neighbors.reserve(g.adj[v].size());

        for (const auto& edge : g.adj[v]) {
            uint64_t nbr_color = g.colors[edge.to];
            double disc_weight = discretize(edge.weight, config_.epsilon);
            neighbors.emplace_back(edge.type, nbr_color, disc_weight);
        }

        std::sort(neighbors.begin(), neighbors.end());
        Signature sig = {g.colors[v], std::move(neighbors)};
        unique_signatures.insert(sig);
        vertex_signatures[v] = std::move(sig);
    }

    // Canonical assignment: strictly sorted lexicographical order of signatures
    std::map<Signature, uint64_t> canonical_map;
    uint64_t id = 0;
    for (const auto& s : unique_signatures) {
        canonical_map[s] = id++;
    }

    std::vector<uint64_t> next_colors(n);
    for (size_t v = 0; v < n; ++v) {
        next_colors[v] = canonical_map[vertex_signatures[v]];
    }

    g.colors = std::move(next_colors);
}

WLResult WLSolver::run(Graph& g) const {
    WLResult result;
    if (g.num_vertices() == 0) return result;

    auto t_start = std::chrono::high_resolution_clock::now();

    initialize_colors(g);

    int current_classes = count_unique_colors(g);
    result.class_counts_per_iteration.push_back(current_classes);

    if (config_.verbose) {
        std::cout << "[Weighted-WL] Iteration 0 (Initialization): "
                  << current_classes << " classes on " << g.num_vertices() << " vertices\n";
    }

    int iteration = 0;
    int last_classes = 0;

    while (current_classes > last_classes && iteration < static_cast<int>(config_.max_iterations)) {
        iteration++;
        last_classes = current_classes;

        refine_step(g);

        current_classes = count_unique_colors(g);
        result.class_counts_per_iteration.push_back(current_classes);

        if (config_.verbose) {
            std::cout << "[Weighted-WL] Iteration " << iteration << ": "
                      << current_classes << " classes\n";
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    result.elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    result.iterations = iteration;
    result.num_classes = current_classes;
    result.converged = (current_classes == last_classes);
    result.final_colors = g.colors;

    return result;
}

bool WLSolver::are_isomorphic(Graph& g1, Graph& g2) const {
    if (g1.num_vertices() != g2.num_vertices() || g1.num_edges() != g2.num_edges()) {
        return false;
    }

    const size_t n = g1.num_vertices();
    if (n == 0) return true;

    // 1. Joint attribute discretization & canonical assignment
    std::set<double> all_attrs;
    for (size_t v = 0; v < n; ++v) {
        all_attrs.insert(discretize(g1.initial_attributes[v], config_.epsilon));
        all_attrs.insert(discretize(g2.initial_attributes[v], config_.epsilon));
    }

    std::unordered_map<double, uint64_t> attr_to_id;
    uint64_t id = 0;
    for (double attr : all_attrs) {
        attr_to_id[attr] = id++;
    }

    g1.colors.resize(n);
    g2.colors.resize(n);
    for (size_t v = 0; v < n; ++v) {
        g1.colors[v] = attr_to_id[discretize(g1.initial_attributes[v], config_.epsilon)];
        g2.colors[v] = attr_to_id[discretize(g2.initial_attributes[v], config_.epsilon)];
    }

    auto get_color_histogram = [](const Graph& g) {
        std::map<uint64_t, int> hist;
        for (uint64_t c : g.colors) {
            hist[c]++;
        }
        return hist;
    };

    if (get_color_histogram(g1) != get_color_histogram(g2)) {
        return false;
    }

    int last_classes = 0;
    int current_classes = count_unique_colors(g1);

    using NeighborTuple = std::tuple<int, uint64_t, double>;
    using Signature = std::pair<uint64_t, std::vector<NeighborTuple>>;

    for (size_t it = 0; it < config_.max_iterations; ++it) {
        if (current_classes == last_classes) {
            break; // Stabilized
        }
        last_classes = current_classes;

        // Compute signatures for both graphs jointly
        std::vector<Signature> sigs1(n), sigs2(n);
        std::set<Signature> joint_signatures;

        auto compute_signatures = [&](const Graph& g, std::vector<Signature>& sigs) {
            std::vector<NeighborTuple> neighbors;
            for (size_t v = 0; v < n; ++v) {
                neighbors.clear();
                for (const auto& edge : g.adj[v]) {
                    neighbors.emplace_back(edge.type, g.colors[edge.to], discretize(edge.weight, config_.epsilon));
                }
                std::sort(neighbors.begin(), neighbors.end());
                Signature sig = {g.colors[v], std::move(neighbors)};
                joint_signatures.insert(sig);
                sigs[v] = std::move(sig);
            }
        };

        compute_signatures(g1, sigs1);
        compute_signatures(g2, sigs2);

        // Canonical mapping based on sorted signature order
        std::map<Signature, uint64_t> canonical_map;
        uint64_t cid = 0;
        for (const auto& s : joint_signatures) {
            canonical_map[s] = cid++;
        }

        for (size_t v = 0; v < n; ++v) {
            g1.colors[v] = canonical_map[sigs1[v]];
            g2.colors[v] = canonical_map[sigs2[v]];
        }

        if (get_color_histogram(g1) != get_color_histogram(g2)) {
            return false;
        }

        current_classes = count_unique_colors(g1);
    }

    return true;
}

void refine_colors(Graph& g) {
    WLSolver solver;
    solver.refine_step(g);
}

} // namespace weighted_wl
