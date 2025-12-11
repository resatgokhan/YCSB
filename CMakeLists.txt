cmake_minimum_required(VERSION 3.16)
project(ycsbc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(YCSBC_WITH_REDIS "Build Redis binding" ON)
option(YCSBC_WITH_TBB "Build TBB-based bindings" ON)

set(LSM_TREE_DIR "${CMAKE_SOURCE_DIR}/../lsm-tree" CACHE PATH "Path to the lsm-tree project")
set(LSM_TREE_INCLUDE_DIR "${LSM_TREE_DIR}/include" CACHE PATH "lsm-tree include directory")
set(LSM_TREE_LIB "${LSM_TREE_DIR}/build/liblsm-tree.a" CACHE FILEPATH "Built lsm-tree static library (optional)")
set(LSM_TREE_SRC_DIR "${LSM_TREE_DIR}/src" CACHE PATH "lsm-tree source directory for fallback build")

if(NOT EXISTS ${LSM_TREE_INCLUDE_DIR})
    message(FATAL_ERROR "Could not find lsm-tree headers at ${LSM_TREE_INCLUDE_DIR}. "
            "Pass -DLSM_TREE_DIR=/path/to/lsm-tree.")
endif()

set(LSM_TREE_USE_STATIC OFF)
if(EXISTS ${LSM_TREE_LIB})
    set(LSM_TREE_USE_STATIC ON)
endif()

find_package(Threads REQUIRED)

find_package(RocksDB CONFIG QUIET)
if(TARGET RocksDB::rocksdb)
    set(ROCKSDB_LIB RocksDB::rocksdb)
else()
    find_library(ROCKSDB_LIB rocksdb)
endif()

if(NOT ROCKSDB_LIB)
    message(FATAL_ERROR "RocksDB library not found. Install rocksdb (e.g., brew install rocksdb) "
            "or make sure it is visible to the linker.")
endif()

set(TBB_LIB "")
if(YCSBC_WITH_TBB)
    find_package(TBB QUIET)
    if(TARGET TBB::tbb)
        set(TBB_LIB TBB::tbb)
    else()
        find_library(TBB_LIB tbb)
    endif()
    if(NOT TBB_LIB)
        message(WARNING "TBB not found; disabling TBB-backed bindings.")
        set(YCSBC_WITH_TBB OFF)
        set(TBB_LIB "")
    endif()
endif()

set(HIREDIS_LIB "")
set(HIREDIS_INCLUDE_DIR "")
if(YCSBC_WITH_REDIS)
    find_library(HIREDIS_LIB hiredis)
    find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
    if(NOT HIREDIS_LIB OR NOT HIREDIS_INCLUDE_DIR)
        message(WARNING "hiredis not found; disabling Redis binding.")
        set(YCSBC_WITH_REDIS OFF)
        set(HIREDIS_LIB "")
        set(HIREDIS_INCLUDE_DIR "")
    endif()
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

target_compile_definitions(ycsbc PRIVATE
        YCSBC_ENABLE_TBB=$<IF:$<BOOL:${YCSBC_WITH_TBB}>,1,0>
        YCSBC_ENABLE_REDIS=$<IF:$<BOOL:${YCSBC_WITH_REDIS}>,1,0>
)

target_include_directories(ycsbc PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${LSM_TREE_INCLUDE_DIR}
        ${YCSBC_DB_INCLUDE_DIRS}
)

if(YCSBC_WITH_REDIS)
    target_include_directories(ycsbc PRIVATE ${HIREDIS_INCLUDE_DIR})
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

if(YCSBC_WITH_TBB AND TBB_LIB)
    target_link_libraries(ycsbc PRIVATE ${TBB_LIB})
endif()

if(YCSBC_WITH_REDIS AND HIREDIS_LIB)
    target_link_libraries(ycsbc PRIVATE ${HIREDIS_LIB})
endif()
