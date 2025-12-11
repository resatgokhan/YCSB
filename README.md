# YCSB-C

Yahoo! Cloud Serving Benchmark in C++, a C++ version of YCSB (https://github.com/brianfrankcooper/YCSB/wiki)

## Quick Start

To build YCSB-C on Ubuntu, for example:

```
$ sudo apt-get install libtbb-dev
$ make
```

As the driver for Redis is linked by default, change the runtime library path
to include the hiredis library by:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

Run Workload A with a [TBB](https://www.threadingbuildingblocks.org)-based
implementation of the database, for example:
```
./ycsbc -db tbb_rand -threads 4 -P workloads/workloada.spec
```
Also reference run.sh and run\_redis.sh for the command line. See help by
invoking `./ycsbc` without any arguments.

The binary now supports `-load` and `-run` phases; `recordcount` controls how
many records are inserted during load, and `operationcount` controls how many
operations to execute during run. Reference properties files in the workloads
dir.

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
3. Workloads: run `./workloads.sh` to (re)generate the put/get/delete and mixed
   specs under `workloads/` (recordcount/operationcount overrideable via env).
4. Benchmark harness: `./run_ycsb.sh [workload_dir] [log_file]` loads each
   workload into `/tmp/ycsbc_lsm/<db>_<workload>` and runs threads 1/2/4/8
   (repeatable via `REPEAT_NUM`). Override buffer/GC knobs with env vars such
   as `SPLITLSM_BUFFER_SIZE`, `UNIFIEDLSM_WRITE_BUFFER_SIZE`, etc.
