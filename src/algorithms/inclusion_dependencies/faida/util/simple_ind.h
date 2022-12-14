#pragma once

#include "simple_cc.h"

#include <boost/functional/hash_fwd.hpp>
#include <utility>
#include <memory>

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

    std::shared_ptr<SimpleCC> const& left() const { return left_; }
    std::shared_ptr<SimpleCC> const& right() const { return right_; }
};

//TODO улучшить??
template<>
struct std::hash<SimpleIND> {
    size_t operator()(SimpleIND const& ind) const {
        size_t seed = 1337;
        //boost::hash_combine(seed, ind.left());
        //boost::hash_combine(seed, ind.right());
        seed ^= std::hash<SimpleCC>{}(*ind.left());
        seed ^= std::hash<SimpleCC>{}(*ind.right());
        return seed;
    }
};