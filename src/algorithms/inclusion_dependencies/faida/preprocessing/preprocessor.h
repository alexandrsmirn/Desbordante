#pragma once

#include "hashed_column_store.h"

class Preprocessor {
private:
    std::vector<std::unique_ptr<AbstractColumnStore>> stores_;

    //inline static size_t kNullHash = std::hash<std::string>{}(""); //TODO нужно как-то сделать полем
    inline static size_t kNullHash = 0;

    explicit Preprocessor(std::vector<std::unique_ptr<AbstractColumnStore>> && stores)
            : stores_(std::move(stores)) {}
public:
    Preprocessor() = delete;
    Preprocessor(Preprocessor const& other) = delete;
    Preprocessor(Preprocessor && other) = default;
    Preprocessor& operator=(Preprocessor const& other) = delete;
    Preprocessor& operator=(Preprocessor && other) = delete;
    ~Preprocessor() = default;

    std::vector<std::unique_ptr<AbstractColumnStore>> const& GetStores() const { return stores_; }

    static size_t GetNullHash() { return kNullHash; }

    //TODO а нужнно ли динамическое выделение, или просто по значению возвращать??
    static std::unique_ptr<Preprocessor> CreateHashedStores(
        std::string const& dataset_name,
        std::vector<std::unique_ptr<model::IDatasetStream>> const& data_streams,
        int sample_goal);
};
