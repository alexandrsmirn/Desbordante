#include "apriori_candidate_generator.h"

#include <algorithm>
//#include <cassert>
#include <unordered_set>

namespace AprioriCandidateGenerator {

std::shared_ptr<SimpleCC> CombineCCs(SimpleCC const& first, SimpleCC const& second,
                    std::unordered_set<std::shared_ptr<SimpleCC>>& ccs_on_level) {
    //TODO вот тут и стоит сделать ту самую логику с предподсчетом хеша!!
    //assert(first.GetTableNum() == second.GetTableNum());

    std::vector<int> new_columns = first.GetColumnIndices();
    new_columns.push_back(second.GetLastColumn());
    auto [iter, was_inserted] = ccs_on_level.emplace(
            std::make_shared<SimpleCC>(first.GetTableNum(), new_columns));
    return *iter;
}

bool CreateCandidate(SimpleIND const& first, SimpleIND const& second,
                          std::unordered_set<std::shared_ptr<SimpleCC>>& ccs_on_level,
                          std::unordered_set<SimpleIND const*> const& last_result_set,
                          std::vector<SimpleIND>& candidates) {
    //auto new_left = std::make_shared<SimpleCC>(
    //        CombineCCs(*(first.left()), *(second.left()), ccs_on_level));
    //auto new_right = std::make_shared<SimpleCC>(
    //        CombineCCs(*(first.right()), *(second.right()), ccs_on_level));

    auto new_left = CombineCCs(*(first.left()), *(second.left()), ccs_on_level);
    auto new_right = CombineCCs(*(first.right()), *(second.right()), ccs_on_level);

    int const size = new_left->GetColumnIndices().size();

    //auto new_ind = SimpleIND(std::move(new_left), std::move(new_right))
    if (new_left->GetTableNum() == new_right->GetTableNum()) {
        for (int i : new_left->GetColumnIndices()) {
            for (int j : new_right->GetColumnIndices()) {
                if (i == j) return false;
            }
        }
    }

    int const num_checks = size - 2;
    if (num_checks <= 0) {
        candidates.emplace_back(std::move(new_left), std::move(new_right));
        return true;
    }

    std::vector<int> test_left;
    std::vector<int> test_right;
    test_left.reserve(size - 1);
    test_right.reserve(size - 1);

    for (int check = 0; check < num_checks; check++) {
        test_left.clear();
        test_right.clear();
        for (int col_idx = 0; col_idx < size; col_idx++) {
            if (col_idx != check) {
                test_left.push_back(new_left->GetColumnIndices()[col_idx]);
                test_right.push_back(new_right->GetColumnIndices()[col_idx]);
            }
        }
        SimpleIND const qwe = SimpleIND(std::make_shared<SimpleCC>(new_left->GetTableNum(), test_left),
                                  std::make_shared<SimpleCC>(new_right->GetTableNum(), test_right));
        if (last_result_set.find(&qwe) == last_result_set.end()) {
            return false;
        }
    }

    candidates.emplace_back(std::move(new_left), std::move(new_right));
    return true;
}

std::vector<SimpleIND> CreateCombinedCandidates(std::vector<SimpleIND> const& inds) {
    std::vector<SimpleIND const*> last_result(inds.size());
    std::transform(inds.begin(), inds.end(), last_result.begin(),
                   [](SimpleIND const& i) { return &i; });
    std::sort(last_result.begin(), last_result.end(),
              [](SimpleIND const* a, SimpleIND const* b) { return *a < *b; }); //TODO ind comparator. нужна ли сортировка?

    std::unordered_set<SimpleIND const*> last_result_set(last_result.begin(), last_result.end());

    //TODO может все таки сделать полем класса???77???
    std::unordered_set<std::shared_ptr<SimpleCC>> ccs_on_level;
    std::vector<SimpleIND> candidates;
    for (auto left_iter = last_result.begin(); left_iter != last_result.end(); left_iter++) {
        for (auto right_iter = std::next(left_iter); right_iter != last_result.end(); right_iter++) {
            auto const& first_ind = **left_iter;
            auto const& second_ind = **right_iter;

            if (!first_ind.StartsWith(second_ind)) {
                break;
            }

            if (first_ind.left()->GetLastColumn() == second_ind.left()->GetLastColumn()
                || first_ind.right()->GetLastColumn() == second_ind.right()->GetLastColumn()) {
                continue;
            }

            CreateCandidate(first_ind, second_ind, ccs_on_level, last_result_set, candidates);
        }
    }

    return candidates;
}

bool CanBeValid(SimpleIND const& candidate, std::unordered_set<SimpleIND> const& last_results) {
    return true;
}

} // namespace AprioriCandidateGenerator
