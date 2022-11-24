#include "hll_data.h"

bool HLLData::IsIncludedIn(const HLLData& other) const {
    if (!hll_.has_value()){
        return true;
    } else if (!other.GetHll().has_value()) {
        return false;
    } else {
        return hll_->is_included_in(other.GetHll().value());
    }
}
