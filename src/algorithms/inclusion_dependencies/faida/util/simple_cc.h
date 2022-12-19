#pragma once

#include <vector>
#include <memory>

/*
 * Represents a column combination as a vector of the column indices
 */
class SimpleCC {
private:
    int table_num_;
    int index_;
    std::size_t distinct_count_;
    std::vector<int> column_indices_;
    bool is_active_;

    //SimpleCC(int table_num, std::vector<int> col_indices, int additional_col);
public:
    SimpleCC(int table_num, std::vector<int> col_indices)
            : table_num_(table_num), index_(-1), distinct_count_(0),
              column_indices_(std::move(col_indices)), is_active_(true) {}

    SimpleCC(int table_num, std::vector<int> col_indices, int index)
        : table_num_(table_num), index_(index), distinct_count_(0),
          column_indices_(std::move(col_indices)), is_active_(true) {}

    bool operator==(SimpleCC const& other) const {
        return this->table_num_ == other.table_num_
               && this->column_indices_ == other.column_indices_;
    }

    bool operator!=(SimpleCC const& other) const { return !(*this == other); }
    bool StartsWith(SimpleCC const& other) const {
        return std::equal(this->column_indices_.begin(),
                          this->column_indices_.end() - 1,
                          other.column_indices_.begin(),
                          other.column_indices_.end() - 1);
    }

    void SetIndex(int index) { index_ = index; }

    int GetIndex() const { return index_; }
    int GetTableNum() const { return table_num_; }
    int GetLastColumn() const { return column_indices_.back(); }
    std::vector<int> const& GetColumnIndices() const { return column_indices_; }
};

//TODO решение со stackoverflow для хеширования вектора интов.
// мб поменять на вариант с ротацией?
template<>
struct std::hash<SimpleCC> {
    size_t operator()(SimpleCC const& cc) const {
        std::size_t seed = cc.GetColumnIndices().size();
        for (auto i : cc.GetColumnIndices()) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template<>
struct std::hash<std::shared_ptr<SimpleCC>> {
    size_t operator()(std::shared_ptr<SimpleCC> const& cc) const {
        return std::hash<SimpleCC>()(*cc);
    }
};

template<>
struct std::equal_to<std::shared_ptr<SimpleCC>> {
    bool operator()(std::shared_ptr<SimpleCC> const& left, std::shared_ptr<SimpleCC> const& right) const {
        return *left == *right;
    }
};