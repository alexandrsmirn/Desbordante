#include "combined_inclusion_tester.h"

#include "easylogging++.h"

#include "inclusion_dependencies/faida/hashing/hashing.h"

std::vector<int> CombinedInclusionTester::SetCCs(std::vector<std::shared_ptr<SimpleCC>>& combinations) {
    hlls_by_table_.clear();
    std::set<int> active_tables_set;
    int index = 0;
    for (auto const& cc : combinations) {
        active_tables_set.insert(cc->GetTableNum());

        /*
        // тут попробовать мувать СС
        auto& hlls_by_cc = hlls_by_table_[cc.GetTableNum()];
        hlls_by_cc[cc] = CreateApproxDataStructure(cc);
        // или вот так: hlls_by_table_[cc.GetTableNum()][cc] = CreateApproxDataStructure(cc);
         */

        //TODO вообще таблиц в сумме должно быть не очень много, так что hlls_by_table вообше-то
        // можно попробовать как-то оптимизировать на основе этого наблюдения

        //TODO так делаю потому что не уверен что в combinations все cc униклаьны. воозможно я ошибаюсь
        auto& hlls_by_cc = hlls_by_table_[cc->GetTableNum()];
        if (hlls_by_cc.find(cc) == hlls_by_cc.end()) {
            cc->SetIndex(index++);
            hlls_by_cc[cc] = CreateApproxDataStructure(*cc);
        }
    }
    max_id_ = index;
    //TODO мысли. этот метот вызывается один раз в основном цикле, то есть кажется все СС распол
    // заются отсюда, и возможно илх имеет смысл смувить в hlls_by_table?

    return std::vector<int>(active_tables_set.begin(), active_tables_set.end());
}

void CombinedInclusionTester::Initialize(
        std::vector<std::vector<std::vector<size_t>>> const& samples_for_tables) {
    //TODO может просто хранить изначальный список CС, который передаем в SetCCs?
    //TODO ещё в метаноме тут проставляются индексы у комбинаций. почему бы не сделать это ранььше?
    // возможно они пытаются немного упорядочить масстив CC по таблицам, но хз зачем.
    // ещё есть вариант что изначальный список CC содержит дубликаты, и таким способом они пытаются
    // от них избавиться (путем добавления в хешмап)

    //TODO тип индексировать CC по индексам, а не передавать хеши?

    std::vector<size_t> samples; //TODO reserve???
    for (int table = 0; table < (int) samples_for_tables.size(); table++) {
        auto hll_by_cc_iter = hlls_by_table_.find(table);

        if (hll_by_cc_iter != hlls_by_table_.end()) {
            auto const& hll_by_cc = hll_by_cc_iter->second;
            auto const& table_sample = samples_for_tables[table];

            for (std::vector<size_t> const& sample_row : table_sample) {
                for (auto const& [cc, hll_data] : hll_by_cc) {
                    size_t combined_hash = 0;
                    bool is_any_null = false;

                    for (int col_idx : cc->GetColumnIndices()) {
                        size_t value_hash = sample_row[col_idx];
                        if (value_hash == Preprocessor::GetNullHash()) {
                            is_any_null = true;
                            break;
                        }
                        combined_hash = hashing::rotl(combined_hash, 1) ^ value_hash;
                    }

                    if (!is_any_null) {
                        samples.push_back(combined_hash);
                    }
                }
            }
        }
    }
    sampled_inverted_index_.Init(samples, max_id_);
}

void CombinedInclusionTester::InsertRows(std::vector<std::vector<size_t>> const& hashed_cols, int row_idx) {
    //TODO константность???
    auto& hll_by_cc = hlls_by_table_[curr_table_num_];
    unsigned int const chunk_size = hashed_cols[0].size();
    //std::vector<size_t, boost::alignment::aligned_allocator<size_t, 32>> combined_hashes(chunk_size, 0);
    //std::vector<char> nul_combs(chunk_size, 0);

    //TODO в метаноме массив hlls_by_table_!!! возможно это имеет смысл на самом деле, потоиму что
    // метод вызывается для каждой строки таблицы
    for (auto& [cc, hll_data] : hll_by_cc) {
        std::vector<size_t> combined_hashes(chunk_size, 0);
        std::vector<unsigned char> nul_combs(chunk_size, 0);
        //auto const nullhashes = _mm256_set1_epi64x(Preprocessor::GetNullHash());

        for (int const col_idx : cc->GetColumnIndices()) {
            auto const& col_hashes_chunk = hashed_cols[col_idx];

            /*for (unsigned int row_offset = 0; row_offset < chunk_size - chunk_size % 4; row_offset += 4) {
                auto const hashes = _mm256_load_si256((__m256i*) (&col_hashes_chunk[row_offset]));
                auto const comb_hashes = _mm256_load_si256((__m256i*) (&combined_hashes[row_offset]));

                auto const cmp_res = _mm256_cmpeq_epi64(hashes, nullhashes);
                unsigned int const cmpResultMask = _mm256_movemask_epi8(cmp_res); //имеем 4 байта. ff, равен нулл хешу
                (unsigned int&) *(&nul_combs[row_offset]) |= cmpResultMask;

                auto const qwe1 = _mm256_slli_epi64(comb_hashes, 1);
                auto const qwe2 = _mm256_srli_epi64(comb_hashes, 63);
                auto const rotated_hashes = _mm256_or_si256(qwe1, qwe2);
                auto const xored = _mm256_xor_si256(rotated_hashes, hashes);

                _mm256_store_si256((__m256i*) (&combined_hashes[row_offset]), xored);
            }

            for (unsigned int row_offset = chunk_size - chunk_size % 4; row_offset < chunk_size; row_offset++) {
                auto const hash = col_hashes_chunk[row_offset];
                auto const comb_hash = combined_hashes[row_offset];

                unsigned char const cmp_res = hash == Preprocessor::GetNullHash() ? 0xFF : 0;
                nul_combs[row_offset] |= cmp_res;

                auto const combined_hash = hashing::rotl(comb_hash, 1) ^ hash;
                combined_hashes[row_offset] = combined_hash;
            }*/

            for (unsigned int row_offset = 0; row_offset < chunk_size; row_offset++) {
                auto const hash = col_hashes_chunk[row_offset];
                auto const comb_hash = combined_hashes[row_offset];

                unsigned char const cmp_res = hash == Preprocessor::GetNullHash() ? 0xFF : 0;
                nul_combs[row_offset] |= cmp_res;

                auto const combined_hash = hashing::rotl(comb_hash, 1) ^ hash;
                combined_hashes[row_offset] = combined_hash;
            }
        }

        for (int i = 0; i < chunk_size; i++) {
            size_t const combined_hash = combined_hashes[i];
            bool const has_null = nul_combs[i];
            if (!has_null) {
                if (!sampled_inverted_index_.Update(*cc, combined_hash)) {  // row not found in inverted index (cc is not covered)
                    // so we insert cc into hll if row does not contain null value and is not covered by inv_index
                    InsertRowIntoHLL(*cc, combined_hash, hll_data);
                }
            }
        }
    }
}

void CombinedInclusionTester::StartInsertRow(int table_num) {
    curr_table_num_ = table_num;
}

bool CombinedInclusionTester::IsIncludedIn(std::shared_ptr<SimpleCC> const& dep, std::shared_ptr<SimpleCC> const& ref) {
    //TODO возможно вставка null-ов где-то выше в метантме нужна именно тут
    if (hlls_by_table_[ref->GetTableNum()].find(ref) == hlls_by_table_[ref->GetTableNum()].end()
        || hlls_by_table_[dep->GetTableNum()].find(dep) == hlls_by_table_[dep->GetTableNum()].end()) {
        //TODO assert
    }

    bool is_dep_covered = sampled_inverted_index_.IsCovered(dep);
    bool is_ref_covered = sampled_inverted_index_.IsCovered(ref);

    if (is_dep_covered) {
        num_certain_checks_++;
        return sampled_inverted_index_.IsIncludedIn(dep, ref);
    } else if (is_ref_covered) {
        num_certain_checks_++;
        return false;
    } else if (!sampled_inverted_index_.IsIncludedIn(dep, ref)) {
        num_certain_checks_++;
        return false;
    }

    num_uncertain_checks_++;
    return TestWithHLLs(dep, ref);
}
