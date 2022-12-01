#include "preprocessor.h"

#include "easylogging++.h"

#include "column.h"

std::unique_ptr<Preprocessor> Preprocessor::CreateHashedStores(
        std::string const& dataset_name,
        std::vector<std::unique_ptr<model::IDatasetStream>> const& data_streams,
        int sample_goal) {
    auto start_time = std::chrono::system_clock::now();

    std::vector<std::unique_ptr<AbstractColumnStore>> stores;
    stores.reserve(data_streams.size());

    int table_num = 0;
    for (auto& input_data : data_streams) {
        //TODO не помешает ли константность вектоора??
        auto store = HashedColumnStore::CreateFrom(dataset_name, table_num++,
                                                   *input_data, sample_goal);
        stores.emplace_back(std::move(store));
    }

    //TODO
    //Preprocessor::kNullHash = std::hash<std::string>{}("");

    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);
    LOG(INFO) << "preprocessing time:\t" << elapsed_milliseconds.count();
    return std::make_unique<Preprocessor>(Preprocessor(std::move(stores)));
}
