#include "column.h"
#include "hashed_column_store.h"
#include "vertical.h"

std::filesystem::path HashedColumnStore::PrepareDirNext(std::filesystem::path dir,
                                                        int table_id) {
    for (auto const& column : schema->GetColumns()) {
        std::string file_name;
        file_name += std::to_string(table_id);
        file_name += "_";
        //file_name += column->GetName(); //TODO баг: не обрабатываются колонки, у которых есть / в имени
        file_name += std::to_string(column->GetIndex());
        file_name += ".bin";

        std::filesystem::path column_file = dir / file_name;
        column_files_[column->GetIndex()] = std::move(column_file);
        //TODO обработать случаи когда файл уже есть или есть опция переиспользования
    }
    return dir;
}

void HashedColumnStore::WriteColumnsAndSample(model::IDatasetStream& data_stream) {
    bool is_writing_any_column = false;
    std::vector<std::ofstream> column_files_out(schema->GetNumColumns());
    //column_files_out.reserve(column_files_.size());

    int column_idx = 0;
    for (auto const& column_file : column_files_) {
        column_files_out[column_idx++].open(column_file, std::ios::binary);
        is_writing_any_column = true;
    }

    std::vector<std::vector<std::string>> rows_to_sample;
    std::vector<std::optional<size_t>> constant_col_hashes(schema->GetNumColumns(), std::nullopt);
    std::vector<std::unordered_set<size_t>> sampled_col_values(
            schema->GetNumColumns(),
            std::unordered_set<size_t>(kSampleGoal < 0 ? 1000 : kSampleGoal));

    int row_counter = 0;
    while (data_stream.HasNextRow()) {
        std::vector<std::string> row = data_stream.GetNextRow();
        if (row.empty()) { continue; }

        bool is_sample_complete = true;
        bool row_has_unseen_value = false;
        for (unsigned col_idx = 0; col_idx < schema->GetNumColumns(); col_idx++) {
            //TODO попробовать распараллелить колонки по потокам
            std::string const& value = row.at(col_idx);
            size_t value_hash = this->hash(value);

            // TODO подумать над буферизацией. Мб заменить на буффер, и писать sizeof(size_t)*buf_size
            column_files_out[col_idx].write(reinterpret_cast<char*>(&value_hash), sizeof(value_hash));

            if (row_counter == 0) {
                // assume all columns are constant initially
                constant_col_hashes[col_idx] = value_hash;
            } else if (constant_col_hashes[col_idx].has_value()
                       && constant_col_hashes[col_idx].value() != value_hash) {
                constant_col_hashes[col_idx].reset();
            }

            if (value_hash != kNullHash) {
                std::unordered_set<size_t>& sampled_vals = sampled_col_values[col_idx];
                bool const should_sample = kSampleGoal < 0
                    || sampled_vals.size() < static_cast<unsigned>(kSampleGoal);

                is_sample_complete &= !should_sample;
                //row_has_unseen_value |= should_sample && was_inserted;
                if (should_sample && sampled_vals.insert(value_hash).second) {
                    row_has_unseen_value = true;
                }
            }
        }

        if (row_has_unseen_value) {
            rows_to_sample.emplace_back(std::move(row)); //TODO check copying
        }

        row_counter++;

        if (!is_writing_any_column && is_sample_complete) {
            break;
        }
    }
    WriteSample(rows_to_sample);

    for (unsigned col_idx = 0; col_idx < constant_col_hashes.size(); col_idx++) {
        if (constant_col_hashes[col_idx].has_value()) {
            size_t const col_hash = constant_col_hashes[col_idx].value();
            if (col_hash == kNullHash) {
                column_properties[col_idx] = ColumnProperty::NULL_CONSTANT;
            } else {
                column_properties[col_idx] = ColumnProperty::CONSTANT;
            }
        }
    }

    for (auto& col_file : column_files_out) {
        col_file.close();
    }
}

std::unique_ptr<IRowIterator> HashedColumnStore::GetRows(Vertical const& columns) const {
    std::vector<unsigned> const col_indices = columns.GetColumnIndicesAsVector();
    std::vector<std::ifstream> hashed_col_streams(col_indices.size());

    unsigned stream_idx = 0;
    for (unsigned col_idx : col_indices) {
        hashed_col_streams[stream_idx++].open(column_files_[col_idx]);
    }

    //TODO check cast
    return std::make_unique<RowIterator>(std::move(hashed_col_streams));
}

std::unique_ptr<AbstractColumnStore> HashedColumnStore::CreateFrom(
        const std::string& dataset_name,
        int table_num,
        model::IDatasetStream& input_data,
        int sample_goal) {


    auto store = std::make_unique<HashedColumnStore>(HashedColumnStore(input_data.GetNumberOfColumns(), sample_goal));
    store->LoadData(dataset_name, table_num, input_data);

    return store;
}

HashedColumnStore::RowIterator::~RowIterator() {
    for (auto & column_stream : hashed_col_streams_) {
        column_stream.close();
    }
}

bool HashedColumnStore::RowIterator::HasNextBlock() {
    if (!has_next_) return false;

    int const bufsize = 1024;
    std::vector<std::vector<size_t, boost::alignment::aligned_allocator<size_t, 32>>>
            row_hashes_inv(hashed_col_streams_.size(),
                           std::vector<size_t, boost::alignment::aligned_allocator<size_t, 32>>(bufsize));

    /*TODO подумать над буферизацией и распараллеливанием. соотвественно поменяется
     * метод has_next */
    unsigned col_idx = 0;
    //bool last_block = false;
    for (auto& column_stream : hashed_col_streams_) {
        // read the value if there is not eof or any error
        if (!column_stream.read(reinterpret_cast<char*>(row_hashes_inv[col_idx].data()),
                                bufsize * sizeof(size_t))) {
            has_next_ = false;
            row_hashes_inv[col_idx].resize(column_stream.gcount() / sizeof(size_t));
        }
        ++col_idx;
    }

    curr_block_ = std::move(row_hashes_inv);
    return true;
}

std::vector<std::vector<size_t, boost::alignment::aligned_allocator<size_t, 32>>> const& HashedColumnStore::RowIterator::GetNextBlock() {
    return curr_block_;
}
