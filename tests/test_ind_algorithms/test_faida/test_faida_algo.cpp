#include <filesystem>

#include "gtest/gtest.h"

#include "faida/faida.h"

static std::unique_ptr<algos::Faida> CreateFaidaInstance(int sample_goal,
                                                         double hll_accuracy,
                                                         std::filesystem::path const& path,
                                                         char separator = ',',
                                                         bool has_header = true) {
    algos::INDAlgorithm::Config conf{
            .dataset_name = path.filename(),
            .data = std::vector(1, path),
            .separator = separator,
            .has_header = has_header,
            .special_params {
                    {"sample_goal", sample_goal},
                    {"hll_accuracy", hll_accuracy},
                    {"detect_nary", false}}
    };
    return std::make_unique<algos::Faida>(conf);
}

class SingleTableTest : public ::testing::Test {
};

TEST(SingeTableTest, FirsTest) {
    //auto path = std::filesystem::current_path() / ".." / ".." / "input_data" / "WDC_game.csv";
    auto path = std::filesystem::current_path() / ".." / ".." / "tests" / "input_data" / "abalone.csv";
    //std::
    auto algorithm = CreateFaidaInstance(500, 0.01, path, ',', false);
    algorithm->Execute();
}