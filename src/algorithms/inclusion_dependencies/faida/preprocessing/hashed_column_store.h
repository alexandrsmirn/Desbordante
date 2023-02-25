#pragma once

#include "abstract_column_store.h"

class HashedColumnStore : public AbstractColumnStore {
    //using Block = IRowIterator::Block;
private:
    class RowIterator : public IRowIterator {
    private:
        std::vector<std::optional<std::ifstream>> hashed_col_streams_;
        Block curr_block_;
        size_t block_size_;
        bool has_next_;

        //void GetNextIfHas();
    public:
        //TODO описать методы по умолчанию
        explicit RowIterator(std::vector<std::optional<std::ifstream>> && hashed_columns)
            : hashed_col_streams_(std::move(hashed_columns)), block_size_(0), has_next_(true) {}
        ~RowIterator() override;

        bool HasNextBlock() override;
        size_t GetBlockSize() const override { return block_size_; }
        Block const& GetNextBlock() override;
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
    std::unique_ptr<IRowIterator> GetRows(std::unordered_set<int> const& columns) const override;
    static std::unique_ptr<AbstractColumnStore> CreateFrom(std::string const& dataset_name,
                                                           int table_num,
                                                           model::IDatasetStream& data_stream,
                                                           int sample_goal);
};
