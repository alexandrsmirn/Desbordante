#pragma once

#include <iterator>
#include <vector>

#include "boost/align/aligned_allocator.hpp"
#include <optional>
class IRowIterator {
public:
    using AlignedVector = std::vector<size_t, boost::alignment::aligned_allocator<size_t, 32>>;
    using Block = std::vector<std::optional<AlignedVector>>;
    //using iterator_category = std::input_iterator_tag;
    //using value_type = std::vector<size_t>;

    virtual bool HasNextBlock() = 0;
    virtual size_t GetBlockSize() const = 0;
    virtual Block const& GetNextBlock() = 0;

    virtual ~IRowIterator() = default;
};
