//
//  unifiedlsm_db.cc
//  YCSB-C
//

#include "db/unifiedlsm_db.h"

#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "lsm-tree/unified/unified_lsm.h"
#include <rocksdb/options.h>

using std::string;

namespace {

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
  } else {
    const auto fallback = props.GetProperty(prefix + "buffer_size", "");
    if (!fallback.empty()) {
      options.write_buffer_size = std::stoull(fallback);
    }
  }

  return options;
}

} // anonymous namespace

namespace ycsbc {

UnifiedLsmDB::UnifiedLsmDB(const utils::Properties &props) {
  const string db_path = props.GetProperty("unifiedlsm.dbpath", "");
  if (db_path.empty()) {
    throw std::invalid_argument("unifiedlsm.dbpath must be set");
  }

  auto options = BuildOptions(props, "unifiedlsm.");
  lsm_ = std::make_unique<lsm_tree::UnifiedLSM>(db_path, options);
}

UnifiedLsmDB::~UnifiedLsmDB() = default;

int UnifiedLsmDB::Read(const string &table, const string &key,
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

int UnifiedLsmDB::Scan(const string &table, const string &key,
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

int UnifiedLsmDB::Update(const string &table, const string &key,
                         std::vector<KVPair> &values) {
  (void)table;
  try {
    lsm_->Put(key, SerializeValues(values));
    return DB::kOK;
  } catch (const std::exception &) {
    return DB::kErrorNoData;
  }
}

int UnifiedLsmDB::Insert(const string &table, const string &key,
                         std::vector<KVPair> &values) {
  return Update(table, key, values);
}

int UnifiedLsmDB::Delete(const string &table, const string &key) {
  (void)table;
  try {
    lsm_->Delete(key);
    return DB::kOK;
  } catch (const std::exception &) {
    return DB::kErrorNoData;
  }
}

string UnifiedLsmDB::SerializeValues(const std::vector<KVPair> &values) {
  if (values.empty()) return {};
  return values.front().second;
}

void UnifiedLsmDB::PopulateResult(const std::string &value,
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
