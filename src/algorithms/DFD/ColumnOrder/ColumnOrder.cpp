#include "ColumnOrder.h"

#include "OrderedPartition.h"
#include "ColumnLayoutRelationData.h"
#include "RelationalSchema.h"

ColumnOrder::ColumnOrder(ColumnLayoutRelationData const* const relationData)
        : order(relationData->getSchema()->getNumColumns()) {
    std::set<OrderedPartition> partitions;
    for (auto const& columnData : relationData->getColumnData()) {
        partitions.emplace(columnData.getPositionListIndex(), relationData->getNumRows(), columnData.getColumn()->getIndex());
    }

    unsigned int orderIndex = 0;
    for (auto const& partition : partitions) {
        order[orderIndex++] = partition.getColumnIndex();
    }
}

std::vector<unsigned int> ColumnOrder::getOrderHighDistinctCount(Vertical const& columns) const {
    std::vector<unsigned int> orderForColumns(columns.getArity());

    unsigned int currentOrderIndex = 0;
    for (auto columnIndex : order) {
        if (columns.getColumnIndices()[columnIndex]) {
            orderForColumns[currentOrderIndex++] = columnIndex;
        }
    }

    return orderForColumns;
}

std::vector<unsigned int> ColumnOrder::getOrderLowDistinctCount(Vertical const& columns) const {
    std::vector<unsigned int> orderForColumns(columns.getArity());

    assert(!order.empty());
    unsigned int currentOrderIndex = 0;
    for (unsigned int i = this->order.size() - 1; i >= 0; --i) {
        if (columns.getColumnIndices()[order[i]]) {
            orderForColumns[currentOrderIndex++] = this->order[i];
        }
    }

    return orderForColumns;
}
