//
//  basic_db.cc
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#include "db/db_factory.h"

#include <string>
#include "db/basic_db.h"
#include "db/lock_stl_db.h"
#include "db/splitlsm_db.h"
#include "db/unifiedlsm_db.h"

#ifndef YCSBC_ENABLE_TBB
#define YCSBC_ENABLE_TBB 1
#endif

#ifndef YCSBC_ENABLE_REDIS
#define YCSBC_ENABLE_REDIS 1
#endif

#if YCSBC_ENABLE_REDIS
#include "db/redis_db.h"
#endif

#if YCSBC_ENABLE_TBB
#include "db/tbb_rand_db.h"
#include "db/tbb_scan_db.h"
#endif

using namespace std;
using ycsbc::DB;
using ycsbc::DBFactory;

DB* DBFactory::CreateDB(utils::Properties &props) {
  if (props["dbname"] == "basic") {
    return new BasicDB;
  } else if (props["dbname"] == "lock_stl") {
    return new LockStlDB;
  } else if (props["dbname"] == "redis") {
#if YCSBC_ENABLE_REDIS
    int port = stoi(props["port"]);
    int slaves = stoi(props["slaves"]);
    return new RedisDB(props["host"].c_str(), port, slaves);
#else
    return NULL;
#endif
  } else if (props["dbname"] == "tbb_rand") {
#if YCSBC_ENABLE_TBB
    return new TbbRandDB;
  } else if (props["dbname"] == "tbb_scan") {
    return new TbbScanDB;
#else
    return NULL;
#endif
  } else if (props["dbname"] == "splitlsm") {
    return new SplitLsmDB(props);
  } else if (props["dbname"] == "unifiedlsm") {
    return new UnifiedLsmDB(props);
  } else return NULL;
}
