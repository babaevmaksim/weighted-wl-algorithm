#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

struct Edge {
    int to;
    double weight;
    int type; // 1 - исходящее, -1 - входящее
};

struct Graph {
    std::vector<std::vector<Edge>> adj;
    std::vector<double> colors;
};

#endif