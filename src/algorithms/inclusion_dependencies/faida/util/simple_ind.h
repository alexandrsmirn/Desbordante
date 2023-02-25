#pragma once

#include "simple_cc.h"

#include <boost/functional/hash_fwd.hpp>
#include <utility>
#include <memory>

#include "../hashing/hashing.h"

class SimpleIND {
private:
    std::shared_ptr<SimpleCC> const left_;
    std::shared_ptr<SimpleCC> const right_;
    //TODO может сделать поля указателями?
public:
    SimpleIND(std::shared_ptr<SimpleCC>  left, std::shared_ptr<SimpleCC>  right)
            : left_(std::move(left)), right_(std::move(right)) {}

    bool operator==(SimpleIND const& other) const {
        return *(this->left_) == *(other.left_) && *(this->right_) == *(other.right_);
    }
    bool operator!=(SimpleIND const& other) const { return !(*this == other); }

    bool operator<(SimpleIND const& other) const {
        int diff;
        if (this->left_->GetTableNum() != other.left_->GetTableNum()) {
            return this->left_->GetTableNum() < other.left_->GetTableNum();
        } else if (this->right_->GetTableNum() != other.right_->GetTableNum()) {
            return this->right_->GetTableNum() < other.right_->GetTableNum();
        }

        //assert(this->GetArity() == other.GetArity());
        int const prefix_length = this->GetArity() - 1;
        auto const& this_left_columns = this->left_->GetColumnIndices();
        auto const& this_right_columns = this->right_->GetColumnIndices();
        auto const& other_left_columns = other.left_->GetColumnIndices();
        auto const& other_right_columns = other.right_->GetColumnIndices();

        for (int col_idx = 0; col_idx < prefix_length; col_idx++) {
            if (this_left_columns[col_idx] != other_left_columns[col_idx]) {
                return this_left_columns[col_idx] < other_left_columns[col_idx];
            }
            if (this_right_columns[col_idx] != other_right_columns[col_idx]) {
                return this_right_columns[col_idx] < other_right_columns[col_idx];
            }
        }

        if (this->left_->GetLastColumn() != other.left_->GetLastColumn()) {
            return this->left_->GetLastColumn() < other.left_->GetLastColumn();
        }

        return this->right_->GetLastColumn() < other.right_->GetLastColumn();
    }


    bool StartsWith(SimpleIND const& other) const {
        return this->left_->StartsWith(*other.left_) && this->right_->StartsWith(*other.right_);
    }

    std::shared_ptr<SimpleCC> const& left() const { return left_; }
    std::shared_ptr<SimpleCC> const& right() const { return right_; }
    int GetArity() const { return left_->GetColumnIndices().size(); }
};

//TODO улучшить??
template<>
struct std::hash<SimpleIND> {
    size_t operator()(SimpleIND const& ind) const {
        size_t seed = 0;
        //seed ^= std::hash<SimpleCC>{}(*ind.left());
        //seed ^= std::hash<SimpleCC>{}(*ind.right());
        seed ^= reinterpret_cast<size_t>(ind.left().get());
        seed = hashing::rotl(seed, 1) ^ reinterpret_cast<size_t>(ind.right().get());
        return seed;
    }
};

template<>
struct std::hash<SimpleIND const*> {
    size_t operator()(SimpleIND const* ind) const {
        //return std::hash<SimpleIND>{}(*ind);
        size_t seed = 0;
        seed ^= std::hash<SimpleCC>{}(*(ind->left()));
        seed = hashing::rotl(seed, 11) ^ std::hash<SimpleCC>{}(*(ind->right()));
        return seed;
    }
};

template<>
struct std::equal_to<SimpleIND const*> {
    bool operator()(SimpleIND const* a, SimpleIND const* b) const {
        return *a == *b;
    }
};