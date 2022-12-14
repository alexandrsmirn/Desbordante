#pragma once

#include "inclusion_testing/hll_inclusion_tester.h"
#include "inclusion_testing/iinclusion_tester.h"
#include "ind_algorithm.h"
#include "util/simple_ind.h"
#include "preprocessing/preprocessor.h"

namespace algos {

class Faida : public algos::INDAlgorithm {
private:
    int const kSampleGoal;
    double const kHllAccuracy;
    bool const kDetectNary;
    std::unique_ptr<IInclusionTester> const inclusion_tester_;

    std::vector<SimpleIND> result_;

    // Insert rows into InclusionTester
    void InsertRows(std::vector<int> const& active_tables, Preprocessor const& data);
    std::vector<SimpleIND> TestCandidates(std::vector<SimpleIND> const& candidates);

public:
    /*Faida(std::vector<model::IDatasetStream> data_streams,
          std::vector<std::string_view> phase_names, int sample_goal)
            : algos::INDAlgorithm(std::move(data_streams),std::move(phase_names)),
              kSampleGoal(sample_goal) {}*/

    Faida(Config const& config, std::vector<std::string_view> phase_names)
        : INDAlgorithm(config, std::move(phase_names)),
          kSampleGoal(config.GetSpecialParam<int>("sample_goal")),
          kHllAccuracy(config.GetSpecialParam<double>("hll_accuracy")),
          kDetectNary(config.GetSpecialParam<bool>("detect_nary")),
          inclusion_tester_(std::make_unique<HllInclusionTester>(kHllAccuracy)) {}

    explicit Faida(Config const& config)
        : Faida(config, {"IND mining"}) {}

    std::vector<SimpleIND> const& GetResult() const { return result_; }

    unsigned long long Execute() override;

    std::vector<std::shared_ptr<SimpleCC>> CreateUnaryCCs(Preprocessor const& data) const;
    std::vector<SimpleIND> CreateUnaryINDCandidates(std::vector<std::shared_ptr<SimpleCC>> const& combinations) const;
};

} // namespace algos