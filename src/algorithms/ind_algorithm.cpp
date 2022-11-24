#include "ind_algorithm.h"

namespace algos {

INDAlgorithm::INDAlgorithm(INDAlgorithm::Config const& config,
                           std::vector<std::string_view> phase_names)
        : Primitive(std::move(phase_names)), config_(config) {
    data_streams_.reserve(config.data.size());
    for (auto const& file : config.data) {
        data_streams_.emplace_back(
                std::make_unique<CSVParser>(file, config.separator, config.has_header));
    }
}

} // namespace algos
