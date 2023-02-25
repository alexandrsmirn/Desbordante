#include "sampled_inverted_index.h"

#include "easylogging++.h"

void SampledInvertedIndex::Init(std::vector<size_t> const& sampled_hashes, int max_id) {
    int const bucket_count = 4; //TODO подумать сколько тут сделать
    for (size_t combined_hash : sampled_hashes) {
        inverted_index_.try_emplace(combined_hash, std::unordered_set<int>(bucket_count));
        //inverted_index_.try_emplace(combined_hash, emhash2::HashSet<int>(4));
    }
    max_id_ = max_id;
    /*seen_cc_indices_.resize(max_id);
    seen_cc_indices_.clear();
    non_covered_cc_indices_.resize(max_id);
    non_covered_cc_indices_.clear();*/

    seen_cc_indices_ = boost::dynamic_bitset<>(max_id);
    non_covered_cc_indices_ = boost::dynamic_bitset<>(max_id);

    discovered_inds_.clear();
}

void SampledInvertedIndex::FinalizeInsertion(
        std::unordered_map<int, std::unordered_map<std::shared_ptr<SimpleCC>, HLLData>> const& hlls_by_table) {
    //std::unordered_map<std::shared_ptr<SimpleCC>, std::vector<int>> ref_by_dep_ccs(max_id_ + 1); //TODO ptr hash??? и хз насчет размера
    std::unordered_map<int, std::vector<int>> ref_by_dep_ccs(max_id_ + 1); //TODO ptr hash??? и хз насчет размера
    std::vector<std::shared_ptr<SimpleCC>> column_combinations(max_id_ + 1);

    for (auto const& [table_num, hlls_by_cc] : hlls_by_table) {
        for (auto const& [cc, ints] : hlls_by_cc) {
            column_combinations[cc->GetIndex()] = cc;
            //TODO!!! ref_by_dep_ccs -- хз что делать с нуллами.
            // видимо эта строчка даже не нужна, поскольку в get ниже мы все равно получили бы null
            // вне зависимости от того, добавилили ли мы его тут или нет (но хз).
            // возможно это нужно для того чтобы зарезервить место в хешмапе?
        }
    }
    //LOG(INFO) << "Finalize: init CCs";

    for (auto const& [hash, cc_indices] : inverted_index_) {
        //TODO возмонжо есть коллизии? узнать, зачем такой биндинг
        for (int const dep_cc_index : cc_indices) {
            seen_cc_indices_.set(dep_cc_index);
            auto ref_ccs_iter = ref_by_dep_ccs.find(dep_cc_index);

            if (ref_ccs_iter == ref_by_dep_ccs.end()) {
                // if value is unseen
                std::vector<int> ref_ccs;
                ref_ccs.reserve(cc_indices.size() - 1);
                for (int const ref_cc_index : cc_indices) {
                    if (dep_cc_index != ref_cc_index) {
                        ref_ccs.push_back(ref_cc_index);
                    }
                }
                ref_by_dep_ccs[dep_cc_index] = std::move(ref_ccs);
            } else if (!ref_ccs_iter->second.empty()) {
                std::vector<int>& ref_ccs = ref_ccs_iter->second;
                //TODO если сделаем значение у inverted_index как упорядоченный map, то можно
                // попробовать поработать с упорядоченным ретейном

                //resf_ccs.RetainAll(cc_indices);
                auto const& value_group = cc_indices;
                ref_ccs.erase(std::remove_if(
                                  ref_ccs.begin(),
                                  ref_ccs.end(),
                                  [&value_group] (int cc_id) {
                                      return value_group.find(cc_id) == value_group.end();
                                  }),
                              ref_ccs.end());
                //TODO возможно тупо заменить на копирование массива без ненужных элементов?
            }
        }
    }

    inverted_index_.clear();
    //LOG(INFO) << "Finalize: almost done";

    //LOG(INFO) << ref_by_dep_ccs.size();
    //LOG(INFO) << ref_by_dep_ccs.bucket_count();
    // Materialize the INDs
    //size_t size = 0;
    for (auto const& [lhs, rhss] : ref_by_dep_ccs) {
        // TODO вот тут вероятно какая-то хрень со вставклй null-ов в самом начале была
        // if (rhss == null) contionue;
        //LOG(INFO) << lhs->GetIndex();
        for (int const rhs : rhss) {
            //auto qwe = SimpleIND(*lhs, *(column_combinations[rhs]));
            //TODO снова чек копирование
            // emplace?
            //discovered_inds_.insert(SimpleIND(column_combinations[lhs], column_combinations[rhs]));
            discovered_inds_.emplace(column_combinations[lhs], column_combinations[rhs]);
        }
        //size_t qwe = discovered_inds_.size();
        //if (qwe - size > 500000) {
        //    LOG(INFO) << qwe;
        //    size = qwe;
        //}
    }
    //LOG(INFO) << "Finalize: done";
}
