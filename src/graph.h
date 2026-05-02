#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

struct Edge {
    int to;
    double weight;
};

struct Graph {
    std::vector<std::vector<Edge>> adj; // Список с соседями и рёбрами к ним для каждой вершины в формате: adj[0] — содержит все рёбра (с примыкающим узлом-соседом), выходящие из города №0.
    std::vector<int> colors;
};

#endif