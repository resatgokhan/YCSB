set(_db_src_dir ${CMAKE_CURRENT_SOURCE_DIR})

set(YCSBC_DB_SOURCES
        ${_db_src_dir}/db_factory.cc
        ${_db_src_dir}/splitlsm_db.cc
        ${_db_src_dir}/unifiedlsm_db.cc
)

set(YCSBC_DB_SOURCES ${YCSBC_DB_SOURCES} PARENT_SCOPE)

set(YCSBC_DB_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${LSM_TREE_INCLUDE_DIR}
        PARENT_SCOPE
)
