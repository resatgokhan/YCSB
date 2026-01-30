# Changes Overview

- Added SplitLSM and UnifiedLSM bindings for YCSB-C (`db/splitlsm_db.*`, `db/unifiedlsm_db.*`) that map YCSB read/put/delete/scan operations onto the lsm-tree engines with configurable RocksDB options and GC/memtable knobs.
- Enabled delete operations in the YCSB workload and client flow (`core/core_workload.{h,cc}`, `core/client.h`) and extended CLI parsing to support `-load/-run` phases and inline `-p key=value` properties (`ycsbc.cc`).
- Introduced CMake-based build that links against the sibling `lsm-tree` project and RocksDB, with optional Redis/TBB toggles (`CMakeLists.txt`, `db/CMakeLists.txt`); updated `.gitignore` accordingly.
- Added workload specs for pure put/get/delete and mixed ratios (`workloads/`).
- Added automated benchmark harness (`run_ycsb.sh`, `run.sh` wrapper) to load and run all workloads for both engines across thread counts, logging to `ycsbc.output`.
- Updated README with build/test instructions, new bindings, delete proportion support, and harness usage.
