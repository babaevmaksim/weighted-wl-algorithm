#include <iostream>
#include "graph.h"

void refine_colors(Graph& g);

int main() {
    // Граф А
    Graph graphA;
    
    // делаем 3 вершины (0, 1, 2)
    graphA.adj.resize(3);
    
    // добавляем рёбра и веса для них:  0 -> 1 (вес 1.0), 0 -> 2 (вес 1.0)
    graphA.adj[0].push_back({1, 1.0}); 
    graphA.adj[0].push_back({2, 1.0});
    
    // 1 -> 0, 1 -> 2
    graphA.adj[1].push_back({0, 1.0});
    graphA.adj[1].push_back({2, 1.0});
    
    // 2 -> 0, 2 -> 1
    graphA.adj[2].push_back({0, 1.0});
    graphA.adj[2].push_back({1, 1.0});

    // начальный цвет "1"
    graphA.colors = {1, 1, 1};


    // Граф B, аналогично
    Graph graphB;
    graphB.adj.resize(3);

    // вес 5.0
    graphB.adj[0].push_back({1, 5.0}); 
    graphB.adj[0].push_back({2, 1.0});

    graphB.adj[1].push_back({0, 5.0});
    graphB.adj[1].push_back({2, 1.0});

    graphB.adj[2].push_back({0, 1.0});
    graphB.adj[2].push_back({1, 1.0});

    graphB.colors = {1, 1, 1};

    refine_colors(graphA);
    refine_colors(graphB);

    std::cout << "Graph A (Uniform weights) colors: ";
    for (int c : graphA.colors) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    std::cout << "Graph B (Different weights) colors: ";
    for (int c : graphB.colors) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    return 0;
}
