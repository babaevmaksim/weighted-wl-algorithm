#include "graph.h"
#include <map>
#include <vector>
#include <algorithm>

void refine_colors(Graph& g) { // g - имя для графа, & - вместо того чтобы создавать физическую копию всего графа, мы просто создаем ссылку на него
    
    // std::pair<int, double> - цвет соседнего узла и вес ребра, которая к нему ведёт;
    // std::vector<std::pair<int, double>> - соответственно список цветов соседних узлов и весов рёбер, которые к ним ведут
    // std::map<std::vector<std::pair<int, double>>, int> - цвет для соответствующего набора цветов соседних узлов и весов рёбер, которые к ним ведут
    
    std::map<std::vector<std::pair<int, double>>, int> color_map; 
    std::vector<int> next_colors(g.adj.size()); // здесь сохраняем новые цвета для узлов, чтобы сразу их не менять в графе, а поменять только после того, как мы пройдём все узлы

    for (int v = 0; v < g.adj.size(); ++v) {
        std::vector<std::pair<int, double>> neighbors; // сюда будем собирать цвет соседних узлов и вес рёбер к ним для узла v
        for (auto& edge : g.adj[v]) { // проходимся по каждому соседнему узлу и ребра к ней для вершины v
            neighbors.push_back({g.colors[edge.to], edge.weight}); // добавляем в neighbors цвет соседнего узла и вес ребра для v
        }
        std::sort(neighbors.begin(), neighbors.end()); // сортируем, т. к. для графа неважно, в каком порядки записаны соседние узлы, поэтому нам надо сделать мультимножества одинаковыми

        if (color_map.find(neighbors) == color_map.end()) { // условие, что в color_map не содержится neighbors
            color_map[neighbors] = color_map.size(); // добавляем в color_map новый набор и присваиваем цвет (int) размер color_map (он на 1 больше)
        }
        next_colors[v] = color_map[neighbors]; // записываем новый цвет для узла v
    }
    g.colors = next_colors; // меняем цвета одновременно для всех узлов
}