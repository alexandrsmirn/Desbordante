#pragma once

#include <unordered_map>
#include <unordered_set>

//#include "hash_set4.hpp"
#include "hash_set2.hpp" // -- наилучший вариант сета

//#include "hash_table8.hpp" тоже было норм , вроде как по бенчмаркам говорят быстрая итерация
//emhash7 table быстрее работает, но там муть со structure binding и бакетами, нет ли лишнего??
//у хештейбла из emhash5 лучший результат, но почему-то неправильные ответы! обработка коллизий?
#include "hash_table7.hpp"

#include <boost/dynamic_bitset.hpp>

#include "hll_data.h"
#include "../util/simple_ind.h"

class SampledInvertedIndex {
private:
    /* Maps combined hash to the column conbination IDs */
    // TODO it seems inv_index is filled only once in Init()
    //std::unordered_map<size_t, std::unordered_set<int>> inverted_index_;
    emhash7::HashMap<size_t, emhash2::HashSet<int>> inverted_index_;
    std::unordered_set<SimpleIND> discovered_inds_;

    boost::dynamic_bitset<> seen_cc_indices_; //TODO название???
    boost::dynamic_bitset<> non_covered_cc_indices_;

    int max_id_;

public:
    SampledInvertedIndex()
            : max_id_(0) {}

    void Init(std::vector<size_t> const& sampled_hashes, int max_id);
    bool Update(SimpleCC const& combination, size_t hash); //TODO maybe private?

    bool IsCovered(std::shared_ptr<SimpleCC> const& combination) {
        return !non_covered_cc_indices_.test(combination->GetIndex());
    }

    bool IsIncludedIn(std::shared_ptr<SimpleCC> const& a, std::shared_ptr<SimpleCC> const& b) {
        return !seen_cc_indices_.test(a->GetIndex())
               || discovered_inds_.find(SimpleIND(a, b)) != discovered_inds_.end();
        //TODO проверить копирование
    }

    void FinalizeInsertion(std::unordered_map<int, std::unordered_map<std::shared_ptr<SimpleCC>, HLLData>> const& hlls_by_table);
};
