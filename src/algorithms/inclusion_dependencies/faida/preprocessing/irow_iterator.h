#pragma once

#include <iterator>
#include <vector>

//#include "boost/align/aligned_allocator.hpp"

class IRowIterator {
public:
    //using iterator_category = std::input_iterator_tag;
    //using value_type = std::vector<size_t>;

    virtual bool HasNextBlock() = 0;
    virtual std::vector<std::vector<size_t>> const& GetNextBlock() = 0;

    virtual ~IRowIterator() = default;
};
