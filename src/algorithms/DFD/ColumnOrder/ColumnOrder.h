#pragma once

#include "Vertical.h"
#include "ColumnData.h"

class ColumnOrder {
private:
    std::vector<unsigned int> order;
public:
    explicit ColumnOrder(ColumnLayoutRelationData const* const relationData);
    ColumnOrder() = default;

    std::vector<unsigned int> getOrderHighDistinctCount(Vertical const& columns) const;
    std::vector<unsigned int> getOrderLowDistinctCount(Vertical const& columns) const;
};
