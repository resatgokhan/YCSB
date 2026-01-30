//
//  basic_db.cc
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#include "db/db_factory.h"

#include <string>
#include "db/splitlsm_db.h"
#include "db/unifiedlsm_db.h"

using namespace std;
using ycsbc::DB;
using ycsbc::DBFactory;

DB* DBFactory::CreateDB(utils::Properties &props) {
  if (props["dbname"] == "splitlsm") {
    return new SplitLsmDB(props);
  } else if (props["dbname"] == "unifiedlsm") {
    return new UnifiedLsmDB(props);
  }
  return NULL;
}
