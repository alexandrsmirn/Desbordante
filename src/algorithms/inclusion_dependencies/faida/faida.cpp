#include "faida.h"

#include "easylogging++.h"

#include "candidate_generation/apriori_candidate_generator.h"

namespace algos {

std::vector<std::shared_ptr<SimpleCC>> Faida::CreateUnaryCCs(Preprocessor const& data) const {
    std::vector<std::shared_ptr<SimpleCC>> combinations;
    // TODO combinations.reserve(...)

    int const index = 0;
    for (unsigned table_num = 0; table_num < data.GetStores().size(); table_num++) {
        auto const& store = data.GetStores()[table_num];
        size_t const num_columns = store->GetSchema()->GetNumColumns();

        for (unsigned col_idx = 0; col_idx < num_columns; col_idx++) {
            if (store->IsConstantCol(col_idx) || store->IsNullCol(col_idx)) {
                LOG(INFO) << "got null or constant col " << col_idx;
                continue;
                // TODO добавить опции is_ignore... как в метаноме
            }
            std::vector<int> col_indices(1, col_idx);
            combinations.emplace_back(std::make_shared<SimpleCC>(table_num, std::move(col_indices), index));
        }
    }

    return combinations;
}

std::vector<SimpleIND> Faida::CreateUnaryINDCandidates(
        std::vector<std::shared_ptr<SimpleCC>> const& combinations) const {
    std::vector<SimpleIND> candidates;
    candidates.reserve(combinations.size() * combinations.size());

    for (auto left_it = combinations.begin(); left_it != combinations.end(); left_it++) {
        for (auto right_it = combinations.begin(); right_it != combinations.end(); right_it++) {
            if (left_it != right_it) {
                // candidates.emplace_back(std::move(*left_it), std::move(*right_it));
                // candidates.emplace_back(left_it.base(), right_it.base()); // если поля-указатели?
                // TODO пока простое копирование
                candidates.emplace_back(*left_it, *right_it);
            }
        }
    }

    return candidates;
}

std::vector<std::shared_ptr<SimpleCC>> Faida::ExtractCCs(std::vector<SimpleIND>& candidates) const {
    std::unordered_set<std::shared_ptr<SimpleCC>> combinations;
    for (auto& ind_candidate : candidates) {
        //size_t left_hash = std::hash<std::shared_ptr<SimpleCC>>{}(ind_candidate.left());
        //size_t right_hash = std::hash<std::shared_ptr<SimpleCC>>{}(ind_candidate.right());
        combinations.insert(ind_candidate.left());
        combinations.insert(ind_candidate.right());
    }
    //TODO может тут сразу сет возвращать?
    LOG(INFO) << "Extracted\t" << combinations.size() << " CCs";
    return std::vector<std::shared_ptr<SimpleCC>>(combinations.begin(), combinations.end());
}

unsigned long long Faida::Execute() {
    auto start_time = std::chrono::system_clock::now();
    size_t count = 0;

    // TODO может выделить на стеке??
    std::unique_ptr<Preprocessor> data =
            Preprocessor::CreateHashedStores(config_.dataset_name, data_streams_, kSampleGoal);
    auto const prep_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);
    size_t const prepr_time = prep_milliseconds.count();
    LOG(INFO) << "Preprocessing: \t" << prepr_time;

    std::vector<std::shared_ptr<SimpleCC>> combinations = CreateUnaryCCs(*data);
    // TODO вот тут стоит подумать, как аллоцируем комбинации и зависимости.
    //  делаем ли указатели??
    std::vector<SimpleIND> candidates = CreateUnaryINDCandidates(combinations);
    LOG(INFO) << "Created\t" << candidates.size() << " candidates";

    auto active_tables = inclusion_tester_->SetCCs(combinations);
    InsertRows(active_tables, *data);

    std::vector<SimpleIND> last_result = TestCandidates(candidates);
    count += last_result.size();
    if (kDetectNary) {
        while (!last_result.empty()) {
            // TODO комбинации можно прям вот тут возвращать
            candidates = AprioriCandidateGenerator::CreateCombinedCandidates(last_result);
            if (candidates.empty()) {
                break;
            }
            LOG(INFO) << "Created\t" << candidates.size() << " candidates";
            combinations = ExtractCCs(candidates); //TODO может возвращать хеш таблицу?
            active_tables = inclusion_tester_->SetCCs(combinations);
            InsertRows(active_tables, *data);

            last_result = TestCandidates(candidates);
            LOG(INFO) << "Found\t" << last_result.size() << " INDs on this level";
            count += last_result.size();
        }
    }

    auto const elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);
    long long millis = elapsed_milliseconds.count();

    LOG(INFO) << "\nсertain:\t" << inclusion_tester_->GetNumCertainChecks();
    LOG(INFO) << "uncertain:\t" << inclusion_tester_->GetNumUncertainChecks();
    LOG(INFO) << "time:\t" << millis;
    LOG(INFO) << "\tPreprocessong:\t" << prepr_time;
    LOG(INFO) << "\tInserting:\t" << insert_time_;
    LOG(INFO) << "\tChecking:\t" << check_time_;
    LOG(INFO) << "ind count:\t" << count;

    return millis;
}

void Faida::InsertRows(std::vector<int> const& active_tables, Preprocessor const& data) {
    using std::vector;
    auto start_time = std::chrono::system_clock::now();
    // std::vector<std::vector<std::vector<size_t>>> samples;
    vector<vector<vector<size_t>>> samples;
    samples.reserve(data.GetStores().size());

    for (auto const& store : data.GetStores()) {
        samples.emplace_back(store->GetSample());
    }
    inclusion_tester_->Initialize(samples);

    for (int curr_table : active_tables) {
        // TODO: We always read all columns, even if we don't need to.
        AbstractColumnStore const* const table_store = data.GetStores()[curr_table].get();
        auto input_iter = table_store->GetRows();

        inclusion_tester_->StartInsertRow(curr_table);
        int row_idx = 0;
        while (input_iter->HasNextBlock()) {
            //auto row = input_iter->GetNext();
            //inclusion_tester_->InsertRow(row, row_idx++);
            inclusion_tester_->InsertRows(input_iter->GetNextBlock(), row_idx++);
        }
        LOG(INFO) << "num rows:\t" << row_idx;
    }

    LOG(INFO) << "Start finalize insertion";
    inclusion_tester_->FinalizeInsertion();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);
    size_t const millis = elapsed_milliseconds.count();
    insert_time_ += millis;
    LOG(INFO) << "Insert rows time:\t" << millis;
}

std::vector<SimpleIND> Faida::TestCandidates(std::vector<SimpleIND> const& candidates) {
    auto start_time = std::chrono::system_clock::now();
    std::vector<SimpleIND> result;

    // unsigned candidate_count = 0;
    for (SimpleIND const& candidate_ind : candidates) {
        if (inclusion_tester_->IsIncludedIn(candidate_ind.left(), candidate_ind.right())) {
            result.emplace_back(candidate_ind);
            //LOG(INFO) << candidate_ind.left().GetColumnIndices()[0] << ' ' << candidate_ind.right().GetColumnIndices()[0];
        }
    }

    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);
    size_t const millis = elapsed_milliseconds.count();
    check_time_ += millis;
    LOG(INFO) << "Candidates check time:\t" << millis;
    return result;
}

} // namespace algos
