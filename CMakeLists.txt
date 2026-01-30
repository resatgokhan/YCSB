cmake_minimum_required(VERSION 3.16)
project(ycsbc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LSM_TREE_DIR "${CMAKE_SOURCE_DIR}/../lsm-tree" CACHE PATH "Path to the lsm-tree project")
set(LSM_TREE_INCLUDE_DIR "${LSM_TREE_DIR}/include" CACHE PATH "lsm-tree include directory")
set(LSM_TREE_LIB "${LSM_TREE_DIR}/build/liblsm-tree.a" CACHE FILEPATH "Built lsm-tree static library (optional)")
set(LSM_TREE_SRC_DIR "${LSM_TREE_DIR}/src" CACHE PATH "lsm-tree source directory for fallback build")
set(ROCKSDB_LIB "${LSM_TREE_DIR}/build/vendor/rocksdb/lib/librocksdb.a" CACHE FILEPATH "Path to RocksDB library (optional)")
set(ROCKSDB_INCLUDE_DIR "${LSM_TREE_DIR}/build/vendor/rocksdb/include" CACHE PATH "Path to RocksDB includes (optional)")

if(NOT EXISTS ${LSM_TREE_INCLUDE_DIR})
    message(FATAL_ERROR "Could not find lsm-tree headers at ${LSM_TREE_INCLUDE_DIR}. "
            "Pass -DLSM_TREE_DIR=/path/to/lsm-tree.")
endif()

set(LSM_TREE_USE_STATIC OFF)
if(EXISTS ${LSM_TREE_LIB})
    set(LSM_TREE_USE_STATIC ON)
endif()

find_package(Threads REQUIRED)

find_library(URING_LIB uring)

if(ROCKSDB_LIB AND EXISTS ${ROCKSDB_LIB})
    message(STATUS "Using RocksDB library at ${ROCKSDB_LIB}")
else()
    find_package(RocksDB CONFIG QUIET)
    if(TARGET RocksDB::rocksdb)
        set(ROCKSDB_LIB RocksDB::rocksdb)
    else()
        find_library(ROCKSDB_LIB rocksdb)
    endif()
endif()

if(NOT ROCKSDB_LIB)
    message(FATAL_ERROR "RocksDB library not found. Install rocksdb (e.g., brew install rocksdb) "
            "or make sure it is visible to the linker.")
endif()

add_subdirectory(db)

set(YCSBC_SOURCES
        ycsbc.cc
        core/core_workload.cc
)

add_executable(ycsbc
        ${YCSBC_SOURCES}
        ${YCSBC_DB_SOURCES}
)

target_include_directories(ycsbc PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${LSM_TREE_INCLUDE_DIR}
        ${YCSBC_DB_INCLUDE_DIRS}
)
if(ROCKSDB_INCLUDE_DIR)
    target_include_directories(ycsbc PRIVATE ${ROCKSDB_INCLUDE_DIR})
endif()

if(LSM_TREE_USE_STATIC)
    target_link_libraries(ycsbc PRIVATE ${LSM_TREE_LIB})
else()
    message(STATUS "lsm-tree static library not found at ${LSM_TREE_LIB}; compiling lsm-tree sources directly.")
    set(LSM_TREE_SOURCE_FILES
            ${LSM_TREE_SRC_DIR}/common/base_lsm.cpp
            ${LSM_TREE_SRC_DIR}/lsm_factory.cpp
            ${LSM_TREE_SRC_DIR}/split/garbage_collector.cpp
            ${LSM_TREE_SRC_DIR}/split/split_lsm.cpp
            ${LSM_TREE_SRC_DIR}/split/value_log.cpp
            ${LSM_TREE_SRC_DIR}/unified/unified_lsm.cpp
    )
    target_sources(ycsbc PRIVATE ${LSM_TREE_SOURCE_FILES})
endif()

target_link_libraries(ycsbc PRIVATE
        ${ROCKSDB_LIB}
        Threads::Threads
)

if(URING_LIB)
    target_link_libraries(ycsbc PRIVATE ${URING_LIB})
endif()
