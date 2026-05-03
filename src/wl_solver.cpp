#include "graph.h"
#include <vector>
#include <map>
#include <algorithm>
#include <tuple>

void refine_colors(Graph& g) {

    std::map<std::vector<std::tuple<int, double, double>>, int> color_map;
    std::vector<double> next_colors(g.adj.size());

    for (int v = 0; v < g.adj.size(); ++v) {
  
        std::vector<std::tuple<int, double, double>> neighbors;
        
        for (const auto& edge : g.adj[v]) {
            neighbors.push_back(std::make_tuple(edge.type, g.colors[edge.to], edge.weight));
        }
        
        std::sort(neighbors.begin(), neighbors.end());

        if (color_map.find(neighbors) == color_map.end()) {
            color_map[neighbors] = color_map.size();
        }
        next_colors[v] = (double)color_map[neighbors];
    }

    g.colors = next_colors;
}