#include "graph.h"
#include <vector>
#include <map>
#include <algorithm>
#include <tuple>
#include <cmath> 

// определяем порог чувствительности
const double EPSILON = 1e-14;

auto discretize = [](double value) -> double {
    return std::round(value / EPSILON) * EPSILON;
};

void refine_colors(Graph& g) {
    // определяем новый тип для полной сигнатуры вершины
    // дискретизированный текущий цвет + отсортированный вектор соседей
    using Signature = std::pair<double, std::vector<std::tuple<int, double, double>>>;
    
    std::map<Signature, int> color_map;
    std::vector<double> next_colors(g.adj.size());

    for (int v = 0; v < g.adj.size(); ++v) {
        std::vector<std::tuple<int, double, double>> neighbors;
        
        for (const auto& edge : g.adj[v]) {
            // дискретизация цвета соседа и веса ребра
            double disc_color = discretize(g.colors[edge.to]);
            double disc_weight = discretize(edge.weight);
            
            // тип связи, дискретезированный цвет соседа, дискр. вес
            neighbors.push_back(std::make_tuple(edge.type, disc_color, disc_weight));
        }
        
        // сортировка для независимости от порядка
        std::sort(neighbors.begin(), neighbors.end());
        
        // полная сигнатура (дискр. цвет самого узла + соседи)
        Signature sig = {discretize(g.colors[v]), neighbors};
        
        // проверка, встречалась ли такая сигнатура раньше
        if (color_map.find(sig) == color_map.end()) {
            color_map[sig] = color_map.size();
        }
        
        // кастуем к double по архитектурным соображениям
        next_colors[v] = (double)color_map[sig];
    }
    
    // Обновляем цвета одновременно всех вершин в графе
    g.colors = next_colors;
}