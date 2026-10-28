#include "weighted_wl/graph.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace weighted_wl {

Graph load_from_custom_file(const std::string& filename) {
    std::ifstream file(filename);
    Graph g;
    if (!file.is_open()) {
        std::cerr << "[Error] Could not open file: " << filename << std::endl;
        return g;
    }

    int n = 0, m = 0;
    if (!(file >> n >> m)) {
        std::cerr << "[Error] Invalid format in header (expected n m): " << filename << std::endl;
        return g;
    }

    g.resize(n);

    // Read initial node attributes (colors/balances/features)
    for (int i = 0; i < n; ++i) {
        double val = 0.0;
        if (!(file >> val)) {
            std::cerr << "[Warning] Premature EOF while reading node attributes for vertex " << i << std::endl;
            break;
        }
        g.set_node_attribute(i, val);
    }

    // Read edges: u v weight
    for (int i = 0; i < m; ++i) {
        int u = 0, v = 0;
        double w = 1.0;
        if (!(file >> u >> v >> w)) {
            break;
        }
        g.add_directed_edge(u, v, w);
    }

    return g;
}

bool export_to_dot(const Graph& g, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    // Palette of distinct colors for Graphviz
    const std::vector<std::string> palette = {
        "#E41A1C", "#377EB8", "#4DAF4A", "#984EA3", "#FF7F00",
        "#FFFF33", "#A65628", "#F781BF", "#999999", "#66C2A5",
        "#FC8D62", "#8DA0CB", "#E78AC3", "#A6D854", "#FFD92F"
    };

    out << "digraph WeightedWL {\n";
    out << "  rankdir=LR;\n";
    out << "  node [shape=circle, style=filled, fontname=\"Helvetica\"];\n";
    out << "  edge [fontname=\"Helvetica\", fontsize=10];\n\n";

    for (size_t v = 0; v < g.num_vertices(); ++v) {
        uint64_t c = (v < g.colors.size()) ? g.colors[v] : 0;
        std::string fill = palette[c % palette.size()];
        double attr = (v < g.initial_attributes.size()) ? g.initial_attributes[v] : 0.0;

        out << "  " << v << " [label=\"" << v << " (c=" << c << ")\\nattr=" << attr << "\", fillcolor=\"" << fill << "\"];\n";
    }

    out << "\n";
    for (size_t u = 0; u < g.num_vertices(); ++u) {
        for (const auto& e : g.adj[u]) {
            if (e.type == static_cast<int>(EdgeType::OUTGOING)) {
                out << "  " << u << " -> " << e.to << " [label=\"" << e.weight << "\"];\n";
            }
        }
    }

    out << "}\n";
    return true;
}

} // namespace weighted_wl
