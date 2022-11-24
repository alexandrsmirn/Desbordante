#pragma once

#include <optional>

#include "hyperloglog.h"

class HLLData { //TODO может сделать структурой???
private:
    //hll_t hll_;
    std::optional<hll::HyperLogLog> hll_;

    // is not covered by the inverted index.
    bool is_big_ = false;
    long cardinality_cache_ = -1;

public:
    HLLData()
        : hll_(std::nullopt), is_big_(false) {}

    std::optional<hll::HyperLogLog>& GetHll() { return hll_; }
    std::optional<hll::HyperLogLog> const& GetHll() const { return hll_; }

    void SetHll(hll::HyperLogLog hll) { hll_ = std::move(hll); }
    void SetBig(bool is_big) { is_big_ = is_big; }
    bool IsBig() const { return is_big_; }
    void SetCardinalityCache(long cardinality) { cardinality_cache_ = cardinality; }
    long GetCachedCardinality() const { return cardinality_cache_; }

    bool IsIncludedIn(HLLData const& other) const;

};
