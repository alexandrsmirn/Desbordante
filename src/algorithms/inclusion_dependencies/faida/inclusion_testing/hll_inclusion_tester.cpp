#include "hll_inclusion_tester.h"

void HllInclusionTester::FinalizeInsertion() {
    CombinedInclusionTester::FinalizeInsertion();

    //TODO вроде бы эта часть вообще не нужна, кардиналити у нас нигде не используется
    for (auto& [table_num, hll_by_cc] : hlls_by_table_) {
        for (auto& [cc, hll_data] : hll_by_cc) {
            if (hll_data.IsBig()) {
                hll_data.SetCardinalityCache(round(hll_data.GetHll()->estimate()));
            }
        }
    }
}

void HllInclusionTester::InsertRowIntoHLL(SimpleCC const& cc, size_t row_hash, HLLData& data) {
    //TODO переписать по-нормальному, пока делаю как в метаноме.
    std::optional<hll::HyperLogLog>& hll = data.GetHll();
    if (!hll.has_value()) {
        data.SetHll(hll::HyperLogLog(CalcNumBits(error_)));
    }
    hll->add_hash_32(row_hash);
}
