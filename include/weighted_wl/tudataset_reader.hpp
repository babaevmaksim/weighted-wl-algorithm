#ifndef WEIGHTED_WL_TUDATASET_READER_HPP
#define WEIGHTED_WL_TUDATASET_READER_HPP

#include "graph.hpp"
#include <string>
#include <vector>

namespace weighted_wl {

/**
 * @brief Reads a collection of graphs formatted according to the standard TUDataset benchmark specification.
 * Expects the directory or file prefix to contain:
 *   <prefix>_A.txt
 *   <prefix>_graph_indicator.txt
 *   <prefix>_node_labels.txt (optional)
 *   <prefix>_edge_labels.txt (optional)
 */
class TUDatasetReader {
public:
    static std::vector<Graph> load_dataset(const std::string& dataset_dir, const std::string& dataset_name);
};

} // namespace weighted_wl

#endif // WEIGHTED_WL_TUDATASET_READER_HPP
