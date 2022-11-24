#pragma once

#include <unordered_map>

#include <boost/any.hpp>

#include "primitive.h"

namespace algos {

class INDAlgorithm : public algos::Primitive {
public:
    struct Config {
        using ParamValue = boost::any;
        using ParamsMap = std::unordered_map<std::string, ParamValue>;

        //std::filesystem::path data_folder{}; /* Path to input folder */
        std::string dataset_name;
        std::vector<std::filesystem::path> data;
        //TODO вот эти два поля свои для кадлого файла должны быть
        char separator = ',';                /* Separator for csv */
        bool has_header = true;              /* Indicates if input file has header */

        ParamsMap special_params{}; /* Other special parameters unique for a particular algorithm.
                                     * Use GetSpecialParam() to retrieve parameters by name.
                                     */

        template <typename ParamType>
        ParamType GetSpecialParam(std::string const& param_name) const {
            Config::ParamValue const& value = special_params.at(param_name);
            return boost::any_cast<ParamType>(value);
        }

        bool HasParam(std::string const& param_name) const {
            return special_params.find(param_name) != special_params.end();
        }
    };
protected:
    Config config_;

    //std::vector<std::unique_ptr<model::IDatasetStream>> data_streams_;
    std::vector<std::unique_ptr<model::IDatasetStream>> data_streams_;

public:
    /*INDAlgorithm(std::vector<std::unique_ptr<model::IDatasetStream>> data_streams,
                 std::vector<std::string_view> phase_names)
        : Primitive(std::move(phase_names)), data_streams_(std::move(data_streams)) {}*/

    /*INDAlgorithm(std::vector<model::IDatasetStream> data_streams,
                 std::vector<std::string_view> phase_names)
        : Primitive(std::move(phase_names)), data_streams_(std::move(data_streams)) {

    }*/

    INDAlgorithm(Config const& config, std::vector<std::string_view> phase_names);

    ~INDAlgorithm() override = default;
};

} // namespace algos
