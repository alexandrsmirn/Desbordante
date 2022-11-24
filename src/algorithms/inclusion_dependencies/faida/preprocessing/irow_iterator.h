#pragma once

#include <iterator>
#include <vector>

class IRowIterator {
public:
    //using iterator_category = std::input_iterator_tag;
    //using value_type = std::vector<size_t>;

    virtual bool HasNext() = 0;
    virtual std::vector<size_t> GetNext() = 0;

    virtual ~IRowIterator() = default;
};
