#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../lib/atomicbitvector/include/atomic_bitvector.hpp"

//#include "hash_set4.hpp"
#include "hash_set2.hpp" // -- наилучший вариант сета

//#include "hash_table8.hpp" тоже было норм , вроде как по бенчмаркам говорят быстрая итерация
//emhash7 table быстрее работает, но там муть со structure binding и бакетами, нет ли лишнего??
//у хештейбла из emhash5 лучший результат, но почему-то неправильные ответы! обработка коллизий?
#include "hash_table7.hpp"
#include "hash_table8.hpp"

#include <boost/dynamic_bitset.hpp>

#include "hll_data.h"
#include "../util/simple_ind.h"

class SampledInvertedIndex {
private:
    /* Maps combined hash to the column conbination IDs */
    // TODO it seems inv_index is filled only once in Init()
    //std::unordered_map<size_t, std::unordered_set<int>> inverted_index_;
    emhash7::HashMap<size_t, atomicbitvector::atomic_bv_t> inverted_index_;
    //std::unordered_set<SimpleIND> discovered_inds_;
    emhash2::HashSet<SimpleIND> discovered_inds_;

    boost::dynamic_bitset<> seen_cc_indices_; //TODO название???
    atomicbitvector::atomic_bv_t non_covered_cc_indices_;
    //boost::dynamic_bitset<> non_covered_cc_indices_;

    int max_id_;

public:
    SampledInvertedIndex()
            : max_id_(0), non_covered_cc_indices_(0) {}

    void Init(std::vector<size_t> const& sampled_hashes, int max_id);

    // inline definition for performance reasons
    bool Update(SimpleCC const& combination, size_t hash) {
        auto set_iter = inverted_index_.find(hash);

        if (set_iter == inverted_index_.end()) {
            if (!non_covered_cc_indices_.test(combination.GetIndex())){
                non_covered_cc_indices_.set(combination.GetIndex());
            }
            return false;
        }

        auto& set = set_iter->second;
        if (!set.test(combination.GetIndex())) {
            set.set(combination.GetIndex());
        }
        return true;
    }

    bool IsCovered(std::shared_ptr<SimpleCC> const& combination) {
        return !non_covered_cc_indices_.test(combination->GetIndex());
    }

    bool IsIncludedIn(std::shared_ptr<SimpleCC> const& a, std::shared_ptr<SimpleCC> const& b) {
        return !seen_cc_indices_.test(a->GetIndex())
               || discovered_inds_.find(SimpleIND(a, b)) != discovered_inds_.end();
        //TODO проверить копирование
    }

    void FinalizeInsertion(std::unordered_map<int, emhash8::HashMap<std::shared_ptr<SimpleCC>, HLLData>> const& hlls_by_table);
};
