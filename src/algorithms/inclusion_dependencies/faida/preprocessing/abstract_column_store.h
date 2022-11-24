#pragma once

#include <filesystem>
#include <fstream>

#include "idataset_stream.h"
#include "irow_iterator.h"
#include "relational_schema.h"
#include "vertical.h"

class AbstractColumnStore {
private:
    //std::string const kDatasetName;
    std::string const kDirName = "temp";
    int const kBufSize = 1024 * 1024;

    std::filesystem::path sample_file_;
    unsigned sample_size_ = 0;

protected:
    enum class ColumnProperty : char {
        ORDINARY,
        CONSTANT,
        NULL_CONSTANT
    };

    std::unique_ptr<RelationalSchema> schema;
    int const kSampleGoal;
    std::vector<ColumnProperty> column_properties;
    size_t const kNullHash = std::hash<std::string>{}("");
    //const HashFunction = std::hash<std::string>(); //TODO

    AbstractColumnStore(unsigned num_of_columns, int sample_goal)
        : kSampleGoal(sample_goal),
          column_properties(num_of_columns, ColumnProperty::ORDINARY) {};

    //void Init(std::filesystem::path const& input_file);
    void LoadData(std::string const& dataset_name, int table_num, model::IDatasetStream& input_data);

    std::filesystem::path PrepareDir(std::string const& dataset_name, int table_id);  //TODO нужно ли const метод????
    virtual std::filesystem::path PrepareDirNext(std::filesystem::path dir, int table_id) = 0;

    virtual void WriteColumnsAndSample(model::IDatasetStream& data_stream) = 0;
    void WriteSample(std::vector<std::vector<std::string>> const& rows);

    std::vector<std::vector<size_t>> ReadSample() const;

    size_t hash(std::string const& str) const; //TODO static???
    // TODO get_hash(str, i)

public:
    AbstractColumnStore(AbstractColumnStore && other) = default;

    bool IsConstantCol(int col_idx) const {
        return column_properties[col_idx] == AbstractColumnStore::ColumnProperty::CONSTANT;
    }

    bool IsNullCol(int col_idx) const {
        return column_properties[col_idx] == AbstractColumnStore::ColumnProperty::NULL_CONSTANT;
    }

    RelationalSchema const* GetSchema() const { return schema.get(); }
    std::vector<std::vector<size_t>> GetSample() const { return ReadSample(); }

    virtual std::unique_ptr<IRowIterator> GetRows(Vertical const& columns) const = 0;
    virtual std::unique_ptr<IRowIterator> GetRows() const {
        //todo покраасивее?
        return GetRows(Vertical(schema.get(),
                                boost::dynamic_bitset<>(schema->GetNumColumns()).flip()));
    };

    virtual ~AbstractColumnStore() = default;
};
