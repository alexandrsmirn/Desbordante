#pragma once

#include "../util/simple_cc.h"

class IInclusionTester {
public:
    //TODO может принимать ссылку?
    // вообще можно и мувать, потому что эти комбинации только один раз туда передаются!
    // как раз можно подумать над их аллокацией.
    // !!!почеиу не конст ссылка -- внутри проставляем им id-шники. можно ли это сделать раньше?
    virtual std::vector<int> SetCCs(std::vector<SimpleCC>& combinations) = 0;

    virtual void StartInsertRow(int table_num) = 0;
    virtual void InsertRow(std::vector<std::size_t> const& hashed_row, int row_idx) = 0;

    //TODO видимо имеют пустую по дефолту реализацию в метаноме
    virtual void FinalizeInsertion() = 0;
    virtual void Initialize(std::vector<std::vector<std::vector<std::size_t>>> const& samples) = 0;

    virtual bool IsIncludedIn(SimpleCC const& dep, SimpleCC const& ref) = 0;

    virtual int GetNumCertainChecks() const = 0;
    virtual int GetNumUncertainChecks() const = 0;

    virtual ~IInclusionTester() = default;
};
