set(_db_src_dir ${CMAKE_CURRENT_SOURCE_DIR})

set(YCSBC_DB_SOURCES
        ${_db_src_dir}/db_factory.cc
        ${_db_src_dir}/hashtable_db.cc
        ${_db_src_dir}/splitlsm_db.cc
        ${_db_src_dir}/unifiedlsm_db.cc
)

if(YCSBC_WITH_REDIS)
    list(APPEND YCSBC_DB_SOURCES ${_db_src_dir}/redis_db.cc)
endif()

set(YCSBC_DB_SOURCES ${YCSBC_DB_SOURCES} PARENT_SCOPE)

set(YCSBC_DB_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${LSM_TREE_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../redis
        PARENT_SCOPE
)
