#pragma once

#include "hll_data.h"
#include "iinclusion_tester.h"
#include "../preprocessing/preprocessor.h"
#include "sampled_inverted_index.h"
#include "hash_table8.hpp"

class CombinedInclusionTester : public IInclusionTester {
private:
    SampledInvertedIndex sampled_inverted_index_;
    //std::unordered_map<int, std::unordered_map<SimpleCC, HLLData>> hlls_by_table_;
    //TODO std::vector<std::pair<int, std::unordered_map<SimpleCC, HLLData>>> hlls_by_table_array;

    int curr_table_num_;

    int num_certain_checks_;
    int num_uncertain_checks_;

    int max_id_;

    bool TestWithHLLs(std::shared_ptr<SimpleCC> const& dep, std::shared_ptr<SimpleCC> const& ref) {
        return TestWithHLLs(hlls_by_table_[dep->GetTableNum()][dep],
                            hlls_by_table_[ref->GetTableNum()][ref]);
    }

protected:
    //std::unordered_map<int, std::unordered_map<std::shared_ptr<SimpleCC>, HLLData>> hlls_by_table_;
    std::unordered_map<int, emhash8::HashMap<std::shared_ptr<SimpleCC>, HLLData>> hlls_by_table_;

    virtual HLLData CreateApproxDataStructure(SimpleCC const& combination) = 0;
    virtual void InsertRowIntoHLL(SimpleCC const& cc, size_t row_hash, HLLData& data) = 0;
    virtual bool TestWithHLLs(HLLData const& A, HLLData const& B) = 0;

public:
    CombinedInclusionTester()
            : curr_table_num_(-1), num_certain_checks_(0), num_uncertain_checks_(0), max_id_(-1) {}
    //TODO по хорошему опять же стоит переделать всё в FactoryMethod

    //TODO размышления про ссылки в комментах к IInclusionTester
    std::vector<int> SetCCs(std::vector<std::shared_ptr<SimpleCC>>& combinations) override;
    void Initialize(std::vector<std::vector<std::vector<size_t>>> const& table_samples) override;

    void StartInsertRow(int table_num) override;
    void InsertRows(std::vector<std::vector<size_t>> const& values, int row_idx) override;

    bool IsIncludedIn(std::shared_ptr<SimpleCC> const& dep, std::shared_ptr<SimpleCC> const& ref) override;

    void FinalizeInsertion() override { sampled_inverted_index_.FinalizeInsertion(hlls_by_table_); };

    int GetNumCertainChecks() const override { return num_certain_checks_; }
    int GetNumUncertainChecks() const override { return num_uncertain_checks_; }

    ~CombinedInclusionTester() override = default;
};
