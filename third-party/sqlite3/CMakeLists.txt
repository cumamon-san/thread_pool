project(SQLite3 VERSION 3.41.0 LANGUAGES C)

include(GNUInstallDirs)

#------------------------------------------------------------------------------
# build options and optional modules:
option(SQLITE_ENABLE_COLUMN_METADATA "enables column metadata"                         OFF)
option(SQLITE_ENABLE_DBSTAT_VTAB     "enables dbstat virtual table"                    OFF)
option(SQLITE_ENABLE_FTS3            "enables full text searches version 3"            OFF)
option(SQLITE_ENABLE_FTS4            "enables full text searches version 3 & 4"        OFF)
option(SQLITE_ENABLE_FTS5            "enables full text searches version 5"            OFF)
option(SQLITE_ENABLE_GEOPOLY         "enables Geopoly extention"                       OFF)
option(SQLITE_ENABLE_ICU             "enables international components for unicode"    OFF)
option(SQLITE_ENABLE_MATH_FUNCTIONS  "enables the built-in SQL math functions"         ON)
option(SQLITE_ENABLE_RBU             "enables resumable bulk update extension"         OFF)
option(SQLITE_ENABLE_RTREE           "enables R*TRee index extension"                  OFF)
option(SQLITE_ENABLE_STAT4           "enhances query planner under certain situations" OFF)
option(SQLITE_OMIT_DECLTYPE          "omit declared type of columns"                   ON)
option(SQLITE_OMIT_JSON              "disables JSON SQL functions"                     OFF)
option(SQLITE_RECOMMENDED_OPTIONS    "compile by SQLite3 recommended options"          ON)
option(SQLITE_USE_URI                "enables the default URI filename processing"     OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Release or Debug?" FORCE)
endif()

if(SQLITE_ENABLE_COLUMN_METADATA AND SQLITE_OMIT_DECLTYPE)
    message(FATAL_ERROR "please unset the SQLITE_OMIT_DECLTYPE if you want to\
    compile with SQLITE_ENABLE_COLUMN_METADATA,\
    compiling with both options ON, is not recommended.")
endif()

#------------------------------------------------------------------------------

# SQLite3 as static library:
add_library(${PROJECT_NAME} STATIC sqlite3.c)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME   sqlite3
    PUBLIC_HEADER sqlite3.h
)
target_include_directories(${PROJECT_NAME} PUBLIC $<INSTALL_INTERFACE:include>)
target_compile_definitions(${PROJECT_NAME} PUBLIC # inject user's options
    $<BUILD_INTERFACE:
        $<$<BOOL:${SQLITE_ENABLE_COLUMN_METADATA}>:SQLITE_ENABLE_COLUMN_METADATA>
        $<$<BOOL:${SQLITE_ENABLE_DBSTAT_VTAB}>:SQLITE_ENABLE_DBSTAT_VTAB>
        $<$<BOOL:${SQLITE_ENABLE_FTS3}>:SQLITE_ENABLE_FTS3>
        $<$<BOOL:${SQLITE_ENABLE_FTS4}>:SQLITE_ENABLE_FTS4>
        $<$<BOOL:${SQLITE_ENABLE_FTS5}>:SQLITE_ENABLE_FTS5>
        $<$<BOOL:${SQLITE_ENABLE_GEOPOLY}>:SQLITE_ENABLE_GEOPOLY>
        $<$<BOOL:${SQLITE_ENABLE_ICU}>:SQLITE_ENABLE_ICU>
        $<$<BOOL:${SQLITE_ENABLE_MATH_FUNCTIONS}>:SQLITE_ENABLE_MATH_FUNCTIONS>
        $<$<BOOL:${SQLITE_ENABLE_RBU}>:SQLITE_ENABLE_RBU>
        $<$<BOOL:${SQLITE_ENABLE_RTREE}>:SQLITE_ENABLE_RTREE>
        $<$<BOOL:${SQLITE_ENABLE_STAT4}>:SQLITE_ENABLE_STAT4>
        $<$<BOOL:${SQLITE_OMIT_DECLTYPE}>:SQLITE_OMIT_DECLTYPE>
        $<$<BOOL:${SQLITE_OMIT_JSON}>:SQLITE_OMIT_JSON>
        $<$<BOOL:${SQLITE_USE_URI}>:SQLITE_USE_URI>
        $<$<BOOL:${SQLITE_RECOMMENDED_OPTIONS}>:
            SQLITE_DEFAULT_MEMSTATUS=0
            SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
            SQLITE_DQS=0
            SQLITE_LIKE_DOESNT_MATCH_BLOBS
            SQLITE_MAX_EXPR_DEPTH=0
            SQLITE_OMIT_DEPRECATED
            SQLITE_OMIT_PROGRESS_CALLBACK
            SQLITE_OMIT_SHARED_CACHE
            SQLITE_USE_ALLOCA
        >
    >
)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
