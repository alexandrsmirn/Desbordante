#pragma once

#include "abstract_column_store.h"

class HashedColumnStore : public AbstractColumnStore {
private:
    class RowIterator : public IRowIterator {
    private:
        std::vector<std::ifstream> hashed_col_streams_;
        bool has_more_;

    public:
        //TODO описать методы по умолчанию
        explicit RowIterator(std::vector<std::ifstream> && hashed_columns)
            : hashed_col_streams_(std::move(hashed_columns)), has_more_(true) {}
        ~RowIterator() override;

        bool HasNext() override;
        std::vector<size_t> GetNext() override;
    };


    std::vector<std::filesystem::path> column_files_; //TODO нужно аллоцировать в конструкторе?

    HashedColumnStore(unsigned num_of_columns, int sample_goal)
        : AbstractColumnStore(num_of_columns, sample_goal),
          column_files_(num_of_columns) {}

    std::filesystem::path PrepareDirNext(std::filesystem::path dir, int table_id) override;
protected:
    void WriteColumnsAndSample(model::IDatasetStream& data_stream) override;

public:
    HashedColumnStore(HashedColumnStore && other) = default;

    std::unique_ptr<IRowIterator> GetRows(Vertical const& columns) const override;
    static std::unique_ptr<AbstractColumnStore> CreateFrom(std::string const& dataset_name,
                                                           int table_num,
                                                           model::IDatasetStream& data_stream,
                                                           int sample_goal);
};
