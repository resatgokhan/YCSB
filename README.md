# YCSB-C

Yahoo! Cloud Serving Benchmark in C++, a C++ version of YCSB (https://github.com/brianfrankcooper/YCSB/wiki)

## Quick Start

Only the LSM bindings (SplitLSM/UnifiedLSM) are supported. Build with CMake
against RocksDB and the sibling `lsm-tree` project:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLSM_TREE_DIR=../lsm-tree
cmake --build build
```
Clean build artifacts:
```bash
rm -rf build
```

Example run:
```bash
./build/ycsbc -db splitlsm -threads 4 -load -run \
  -P workloads/workload_put.spec \
  -p splitlsm.dbpath=/tmp/ycsbc_split
```
Use `./run_ycsb.sh` to execute all provided workloads; see `./ycsbc` for CLI help.

## LSM-tree bindings (SplitLSM & UnifiedLSM)

1. Build and sanity-check lsm-tree:
   ```
   cmake -S ../lsm-tree -B ../lsm-tree/build -DCMAKE_BUILD_TYPE=Release
   cmake --build ../lsm-tree/build --target tester
   ./../lsm-tree/build/tester
   ```
2. Build YCSB-C against lsm-tree:
   ```
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLSM_TREE_DIR=../lsm-tree
   cmake --build build
   ```
   - If `../lsm-tree/build/liblsm-tree.a` exists it will be linked; otherwise
     the lsm-tree sources are compiled directly during the YCSB-C build.
   - New db bindings: `-db splitlsm` or `-db unifiedlsm`.
   - Workload properties now accept `deleteproportion` alongside read/update,
     enabling delete-heavy mixes.
   - Key properties (via `-p key=value` or in the spec file):
     - Split: `splitlsm.dbpath` (required), `splitlsm.buffer_size`,
       `splitlsm.collect_size`, `splitlsm.collect_threshold`,
       `splitlsm.punch_hole_threshold`, optional `splitlsm.write_buffer_size`
       for the RocksDB memtable.
     - Unified: `unifiedlsm.dbpath` (required), `unifiedlsm.write_buffer_size`
       (or `unifiedlsm.buffer_size` as a shorthand).
3. Workloads: pick a spec under `workloads/` (recordcount/operationcount
   overrideable via env).
4. Benchmark harness: `./run_ycsb.sh [workload_dir] [log_file]` loads each
   workload into `/tmp/ycsbc_lsm/<db>_<workload>` and runs threads 1/2/4/8
   (repeatable via `REPEAT_NUM`). Override buffer/GC knobs with env vars such
   as `SPLITLSM_BUFFER_SIZE`, `UNIFIEDLSM_WRITE_BUFFER_SIZE`, etc.
