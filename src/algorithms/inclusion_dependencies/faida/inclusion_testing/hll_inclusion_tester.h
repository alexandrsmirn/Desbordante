#pragma once

#include "combined_inclusion_tester.h"

class HllInclusionTester : public CombinedInclusionTester {
private:
    //The desired standard deviation of the relative error of the HLL structures.
    double error_;

    static int CalcNumBits(double error) {
        return int(log((1.106/error) * (1.106 / error)) / log(2));
    }
protected:
    HLLData CreateApproxDataStructure(SimpleCC const& cc) override {
        return HLLData();
    }

    void InsertRowIntoHLL(SimpleCC const& cc, size_t row_hash, HLLData& data) override;

    bool TestWithHLLs(HLLData const& dep_hll, HLLData const& ref_hll) override {
        if (dep_hll.GetCachedCardinality() > ref_hll.GetCachedCardinality()) {
            return false;
        }
        return dep_hll.IsIncludedIn(ref_hll);
    }

    void FinalizeInsertion() override;

public:
    HllInclusionTester(double error)
        : error_(error) {}
};
