#pragma once

#include "simple_cc.h"

#include <boost/functional/hash_fwd.hpp>

class SimpleIND {
private:
    SimpleCC const* const left_;
    SimpleCC const* const right_;
    //TODO может сделать поля указателями?
public:
    SimpleIND(SimpleCC const& left, SimpleCC const& right)
            : left_(&left), right_(&right) {}

    bool operator==(SimpleIND const& other) const {
        return *(this->left_) == *(other.left_) && *(this->right_) == *(other.right_);
    }

    bool operator!=(SimpleIND const& other) const { return !(*this == other); }

    SimpleCC const& left() const { return *left_; }
    SimpleCC const& right() const { return *right_; }
};

//TODO улучшить??
template<>
struct std::hash<SimpleIND> {
    size_t operator()(SimpleIND const& ind) const {
        size_t seed = 1337;
        //boost::hash_combine(seed, ind.left());
        //boost::hash_combine(seed, ind.right());
        seed ^= std::hash<SimpleCC>{}(ind.left());
        seed ^= std::hash<SimpleCC>{}(ind.right());
        return seed;
    }
};