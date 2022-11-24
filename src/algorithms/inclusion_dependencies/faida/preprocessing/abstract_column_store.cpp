#include "abstract_column_store.h"

#include <fstream>

#include "column.h"
#include "../hashing/murmur_hash_3.h"

void AbstractColumnStore::LoadData(std::string const& dataset_name,
                                   int table_num,
                                   model::IDatasetStream& input_data) {
    bool const is_null_eq_null = true; // в оригинале так
    schema = std::make_unique<RelationalSchema>(input_data.GetRelationName(),
                                                is_null_eq_null);
    for (unsigned col_idx = 0; col_idx < input_data.GetNumberOfColumns(); ++col_idx) {
        auto column = Column(schema.get(), input_data.GetColumnName(col_idx), col_idx);
        schema->AppendColumn(std::move(column));
    }
    schema->Init();
    //TODO может вот тут и заполнить массив column_properties

    std::filesystem::path dir = PrepareDir(dataset_name, table_num);

    // abstract method
    WriteColumnsAndSample(input_data);
    //TODO подумать над оберткой как в метаноме
}
std::filesystem::path AbstractColumnStore::PrepareDir(std::string const& dataset_name,
                                                      int table_num) {
    namespace fs = std::filesystem;
    fs::path sample_location_dir = fs::current_path() / kDirName / dataset_name / schema->GetName();

    //TODO обработать исключени?
    fs::create_directories(sample_location_dir);

    std::string sample_file_name;
    sample_file_name += std::to_string(table_num);
    sample_file_name += "-sample.bin";
    sample_file_ = sample_location_dir / sample_file_name;

    return PrepareDirNext(sample_location_dir, table_num);
}

size_t AbstractColumnStore::hash(std::string const& str) const {
    //size_t curr_hash = std::hash<std::string>{}(str);
    uint32_t curr_hash;
    MurmurHash3_x86_32(str.data(), str.size(), 313, &curr_hash);

    if (curr_hash == kNullHash && !str.empty()) {
        curr_hash += 1; // to avoid collision with nullhash
    }

    // TODO реализовать кеширование хешей
    return curr_hash;
}

void AbstractColumnStore::WriteSample(std::vector<std::vector<std::string>> const& rows) {
    sample_size_ = rows.size();
    std::ofstream sample_stream(sample_file_, std::ios::binary);

    //TODO возможно, такая запись будет тормозить
    std::vector<size_t> row_hashes(schema->GetNumColumns());
    for (auto const& row : rows) {
        int col_idx = 0;
        for (auto const& value : row) {
            size_t const value_hash = hash(value);
            row_hashes[col_idx++] = value_hash;
        }
        sample_stream.write(reinterpret_cast<char*>(row_hashes.data()),
                            sizeof(size_t) * row_hashes.size());
    }

    sample_stream.close();
}
std::vector<std::vector<size_t>> AbstractColumnStore::ReadSample() const {
    std::ifstream sample_stream(sample_file_, std::ios::binary);

    //TODO возможно, такое чтение будет тормозить
    std::vector<std::vector<size_t>> hashed_sample(sample_size_);
    for (unsigned row_idx = 0; row_idx < sample_size_; row_idx++) {
        std::vector<size_t> row_hashes(schema->GetNumColumns());
        sample_stream.read(reinterpret_cast<char*>(row_hashes.data()),
                           sizeof(size_t) * row_hashes.size());

        hashed_sample[row_idx] = std::move(row_hashes);
    }

    sample_stream.close();
    return hashed_sample;
}
