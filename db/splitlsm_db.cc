//
//  splitlsm_db.cc
//  YCSB-C
//

#include "db/splitlsm_db.h"

#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "lsm-tree/split/split_lsm.h"
#include <rocksdb/options.h>

using std::string;

namespace {

size_t ParseSizeT(const utils::Properties &props, const string &key, size_t default_value) {
  const string value = props.GetProperty(key, "");
  if (value.empty()) return default_value;
  return static_cast<size_t>(std::stoull(value));
}

rocksdb::Options BuildOptions(const utils::Properties &props, const string &prefix) {
  rocksdb::Options options;
  options.create_if_missing = true;
  options.compression = rocksdb::kNoCompression;

  const auto bg_jobs = props.GetProperty(prefix + "max_background_jobs", "");
  if (!bg_jobs.empty()) {
    options.max_background_jobs = std::stoi(bg_jobs);
  } else {
    options.IncreaseParallelism(static_cast<int>(std::thread::hardware_concurrency()));
  }

  const auto write_buffer = props.GetProperty(prefix + "write_buffer_size", "");
  if (!write_buffer.empty()) {
    options.write_buffer_size = std::stoull(write_buffer);
  }

  return options;
}

} // anonymous namespace

namespace ycsbc {

SplitLsmDB::SplitLsmDB(const utils::Properties &props) {
  const string db_path = props.GetProperty("splitlsm.dbpath", "");
  if (db_path.empty()) {
    throw std::invalid_argument("splitlsm.dbpath must be set");
  }

  const size_t buffer_size = ParseSizeT(props, "splitlsm.buffer_size", 8 * 1024 * 1024);
  const size_t collect_size = ParseSizeT(props, "splitlsm.collect_size", 128 * 1024 * 1024);
  const size_t collect_threshold = ParseSizeT(props, "splitlsm.collect_threshold", collect_size);
  const size_t punch_hole_threshold = ParseSizeT(props, "splitlsm.punch_hole_threshold", collect_size);

  auto options = BuildOptions(props, "splitlsm.");
  lsm_ = std::make_unique<lsm_tree::SplitLSM>(
      db_path, options, buffer_size, collect_size, collect_threshold, punch_hole_threshold);
}

SplitLsmDB::~SplitLsmDB() = default;

int SplitLsmDB::Read(const string &table, const string &key,
                     const std::vector<string> *fields,
                     std::vector<KVPair> &result) {
  (void)table;
  try {
    const auto value = lsm_->Get(key);
    if (value.empty()) return DB::kErrorNoData;
    PopulateResult(value, fields, result);
    return DB::kOK;
  } catch (const std::exception &) {
    return DB::kErrorNoData;
  }
}

int SplitLsmDB::Scan(const string &table, const string &key,
                     int record_count, const std::vector<string> *fields,
                     std::vector<std::vector<KVPair>> &result) {
  (void)table;
  try {
    std::vector<std::pair<string, string>> out;
    out.reserve(record_count);
    lsm_->Scan(key, static_cast<size_t>(record_count), out);
    for (const auto &entry : out) {
      std::vector<KVPair> row;
      PopulateResult(entry.second, fields, row);
      result.emplace_back(std::move(row));
    }
    return DB::kOK;
  } catch (const std::exception &) {
    return DB::kErrorNoData;
  }
}

int SplitLsmDB::Update(const string &table, const string &key,
                       std::vector<KVPair> &values) {
  (void)table;
  try {
    lsm_->Put(key, SerializeValues(values));
    return DB::kOK;
  } catch (const std::exception &) {
    return DB::kErrorNoData;
  }
}

int SplitLsmDB::Insert(const string &table, const string &key,
                       std::vector<KVPair> &values) {
  return Update(table, key, values);
}

int SplitLsmDB::Delete(const string &table, const string &key) {
  (void)table;
  try {
    lsm_->Delete(key);
    return DB::kOK;
  } catch (const std::exception &) {
    return DB::kErrorNoData;
  }
}

string SplitLsmDB::SerializeValues(const std::vector<KVPair> &values) {
  if (values.empty()) return {};
  return values.front().second;
}

void SplitLsmDB::PopulateResult(const std::string &value,
                                const std::vector<std::string> *fields,
                                std::vector<KVPair> &result) {
  if (fields) {
    for (const auto &f : *fields) {
      result.emplace_back(f, value);
    }
  } else {
    result.emplace_back("field0", value);
  }
}

} // namespace ycsbc
