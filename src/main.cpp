#include <iostream>
#include <vector>
#include <set>
#include "graph.h"
#include <fstream>
#include <iostream>

// загрузка
Graph load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    Graph g;
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return g;
    }

    int n, m;
    file >> n >> m;
    g.adj.resize(n);
    g.colors.resize(n);

    // записываем атрибуты (цвета) вершин
    for (int i = 0; i < n; ++i) {
        file >> g.colors[i];
    }

    // записываем ребра
    for (int i = 0; i < m; ++i) {
        int u, v;
        double w;
        file >> u >> v >> w;
        g.adj[u].push_back({v, w, 1});
        g.adj[v].push_back({u, w, -1});
    }
    return g;
}

void refine_colors(Graph& g);

// считаем, сколько разных цветов сейчас в графе
int count_unique_colors(const Graph& g) {
    std::set<int> unique_colors(g.colors.begin(), g.colors.end());
    return unique_colors.size();
}

void run_until_stable(Graph& g) {
    int last_count = 0;
    int current_count = count_unique_colors(g);
    int iteration = 0;

    // пока количество цветов растет - продолжаем
    while (current_count > last_count) {
        iteration++;
        last_count = current_count;
        
        refine_colors(g);
        
        current_count = count_unique_colors(g);
        std::cout << "Iteration " << iteration << ": " << current_count << " colors\n";
    }
}

int main() {

    Graph g = load_from_file("tests/financial_network_pro.txt");
    
    if (g.adj.empty()) return 1;

    std::cout << "Starting refinement..." << std::endl;
    run_until_stable(g);

    std::cout << "Final colors: ";
    for (int c : g.colors) std::cout << c << " ";
    std::cout << std::endl;

    return 0;
}