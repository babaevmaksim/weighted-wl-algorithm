#ifndef WEIGHTED_WL_GRAPH_HPP
#define WEIGHTED_WL_GRAPH_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <iostream>

namespace weighted_wl {

/**
 * @brief Direction of an edge in the adjacency list.
 * OUTGOING (+1): edge directed from current vertex to target vertex.
 * INCOMING (-1): edge directed from target vertex to current vertex.
 */
enum class EdgeType : int {
    INCOMING = -1,
    OUTGOING = 1
};

/**
 * @brief Directed weighted edge representation.
 */
struct Edge {
    int to;          ///< Target vertex index (0-based)
    double weight;   ///< Edge weight (continuous or discrete)
    int type;        ///< EdgeType::OUTGOING (+1) or EdgeType::INCOMING (-1)

    Edge() : to(0), weight(1.0), type(static_cast<int>(EdgeType::OUTGOING)) {}
    Edge(int to_vertex, double w, int t) : to(to_vertex), weight(w), type(t) {}
};

/**
 * @brief Graph with directed weighted edges and continuous/discrete vertex attributes.
 */
class Graph {
public:
    std::vector<std::vector<Edge>> adj;       ///< Adjacency list per vertex
    std::vector<double> initial_attributes;    ///< Initial continuous attributes
    std::vector<uint64_t> colors;             ///< Current partition colors (discrete IDs)

    Graph() = default;
    explicit Graph(size_t num_vertices) {
        resize(num_vertices);
    }

    void resize(size_t num_vertices) {
        adj.resize(num_vertices);
        initial_attributes.resize(num_vertices, 1.0);
        colors.resize(num_vertices, 0);
    }

    size_t num_vertices() const {
        return adj.size();
    }

    size_t num_edges() const {
        size_t count = 0;
        for (const auto& list : adj) {
            for (const auto& e : list) {
                if (e.type == static_cast<int>(EdgeType::OUTGOING)) {
                    count++;
                }
            }
        }
        return count;
    }

    /**
     * @brief Add a directed weighted edge from u to v.
     * Automatically registers the outgoing link in u and incoming link in v.
     */
    void add_directed_edge(int u, int v, double weight) {
        if (u >= static_cast<int>(adj.size()) || v >= static_cast<int>(adj.size())) {
            int max_idx = std::max(u, v);
            resize(max_idx + 1);
        }
        adj[u].push_back(Edge(v, weight, static_cast<int>(EdgeType::OUTGOING)));
        adj[v].push_back(Edge(u, weight, static_cast<int>(EdgeType::INCOMING)));
    }

    /**
     * @brief Add an undirected weighted edge (bidirectional outgoing and incoming).
     */
    void add_undirected_edge(int u, int v, double weight) {
        add_directed_edge(u, v, weight);
        add_directed_edge(v, u, weight);
    }

    void set_node_attribute(int v, double attr) {
        if (v >= static_cast<int>(initial_attributes.size())) {
            resize(v + 1);
        }
        initial_attributes[v] = attr;
    }

    void clear() {
        adj.clear();
        initial_attributes.clear();
        colors.clear();
    }
};

/**
 * @brief Loads a graph from custom text format.
 * Format:
 *   num_vertices num_edges
 *   attr_0 attr_1 ... attr_{n-1}
 *   u_0 v_0 weight_0
 *   ...
 */
Graph load_from_custom_file(const std::string& filename);

/**
 * @brief Exports graph to Graphviz DOT format with color-coded classes.
 */
bool export_to_dot(const Graph& g, const std::string& filename);

} // namespace weighted_wl

#endif // WEIGHTED_WL_GRAPH_HPP
