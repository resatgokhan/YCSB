//
//  splitlsm_db.h
//  YCSB-C
//

#ifndef YCSB_C_SPLITLSM_DB_H_
#define YCSB_C_SPLITLSM_DB_H_

#include "core/db.h"
#include "core/properties.h"

#include <memory>

namespace lsm_tree {
class SplitLSM;
}

namespace ycsbc {

class SplitLsmDB : public DB {
 public:
  explicit SplitLsmDB(const utils::Properties &props);
  ~SplitLsmDB();

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields,
           std::vector<KVPair> &result) override;

  int Scan(const std::string &table, const std::string &key,
           int record_count, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result) override;

  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) override;

  int Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) override;

  int Delete(const std::string &table, const std::string &key) override;

 private:
  std::unique_ptr<lsm_tree::SplitLSM> lsm_;

  static std::string SerializeValues(const std::vector<KVPair> &values);
  static void PopulateResult(const std::string &value,
                             const std::vector<std::string> *fields,
                             std::vector<KVPair> &result);
};

} // namespace ycsbc

#endif //YCSB_C_SPLITLSM_DB_H_
