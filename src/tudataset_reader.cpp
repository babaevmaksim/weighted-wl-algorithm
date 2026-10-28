#include "weighted_wl/tudataset_reader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace weighted_wl {

static std::string join_path(const std::string& dir, const std::string& file) {
    if (dir.empty()) return file;
    char last = dir.back();
    if (last == '/' || last == '\\') {
        return dir + file;
    }
    return dir + "/" + file;
}

std::vector<Graph> TUDatasetReader::load_dataset(const std::string& dataset_dir, const std::string& dataset_name) {
    std::string gi_path = join_path(dataset_dir, dataset_name + "_graph_indicator.txt");
    std::string a_path = join_path(dataset_dir, dataset_name + "_A.txt");
    std::string nl_path = join_path(dataset_dir, dataset_name + "_node_labels.txt");
    std::string el_path = join_path(dataset_dir, dataset_name + "_edge_labels.txt");

    std::ifstream gi_file(gi_path);
    if (!gi_file.is_open()) {
        std::cerr << "[TUDatasetReader] Could not open graph indicator file: " << gi_path << std::endl;
        return {};
    }

    // 1. Read node to graph mapping
    // global_node_id (0-based) -> graph_id (0-based)
    std::vector<int> node_graph;
    std::unordered_map<int, std::vector<int>> graph_to_nodes;

    int gid = 0;
    int global_node_idx = 0;
    while (gi_file >> gid) {
        node_graph.push_back(gid);
        graph_to_nodes[gid].push_back(global_node_idx);
        global_node_idx++;
    }
    gi_file.close();

    int total_graphs = static_cast<int>(graph_to_nodes.size());
    std::vector<Graph> graphs(total_graphs);

    // Map global node index to local node index in its graph
    std::unordered_map<int, int> global_to_local;
    std::unordered_map<int, int> gid_to_pos;

    int pos = 0;
    for (const auto& [id, nodes] : graph_to_nodes) {
        gid_to_pos[id] = pos;
        graphs[pos].resize(nodes.size());
        for (size_t local_idx = 0; local_idx < nodes.size(); ++local_idx) {
            global_to_local[nodes[local_idx]] = static_cast<int>(local_idx);
        }
        pos++;
    }

    // 2. Read node labels/attributes if available
    std::ifstream nl_file(nl_path);
    if (nl_file.is_open()) {
        for (size_t i = 0; i < node_graph.size(); ++i) {
            double label = 0.0;
            if (nl_file >> label) {
                int g_pos = gid_to_pos[node_graph[i]];
                int loc_v = global_to_local[static_cast<int>(i)];
                graphs[g_pos].set_node_attribute(loc_v, label);
            }
        }
        nl_file.close();
    }

    // 3. Read edge list A.txt
    std::ifstream a_file(a_path);
    if (!a_file.is_open()) {
        std::cerr << "[TUDatasetReader] Could not open adjacency file: " << a_path << std::endl;
        return graphs;
    }

    // Also prepare edge labels if present
    std::ifstream el_file(el_path);

    std::string line;
    while (std::getline(a_file, line)) {
        if (line.empty()) continue;
        for (char& c : line) {
            if (c == ',') c = ' ';
        }
        std::istringstream iss(line);
        int u_1based = 0, v_1based = 0;
        if (iss >> u_1based >> v_1based) {
            int u = u_1based - 1;
            int v = v_1based - 1;

            double weight = 1.0;
            if (el_file.is_open()) {
                double el = 1.0;
                if (el_file >> el) {
                    weight = el;
                }
            }

            if (u >= 0 && u < static_cast<int>(node_graph.size()) &&
                v >= 0 && v < static_cast<int>(node_graph.size())) {
                int g1 = node_graph[u];
                int g2 = node_graph[v];
                if (g1 == g2) {
                    int g_pos = gid_to_pos[g1];
                    int loc_u = global_to_local[u];
                    int loc_v = global_to_local[v];
                    graphs[g_pos].add_directed_edge(loc_u, loc_v, weight);
                }
            }
        }
    }

    return graphs;
}

} // namespace weighted_wl
