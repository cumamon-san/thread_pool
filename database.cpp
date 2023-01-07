#include "database.h"

#include <cassert>
#include <stdexcept>
#include <utility>
#include <sqlite3.h>

#include "log.h"

using namespace std::string_literals;

database_t::database_t()
: db_(nullptr)
{
}

database_t::database_t(const std::string& path)
: database_t()
{
    open(path);
}

database_t::~database_t()
{
    if(db_) close();
}

void database_t::open(const std::string& path)
{
    if(db_) close();

    DEBUG("Open " << path);
    if(sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
        throw std::runtime_error("sqlite3_open failed: "s + sqlite3_errmsg(std::exchange(db_, nullptr)));
}

void database_t::close()
{
    DEBUG("Close database");
    if(sqlite3_close(db_) != SQLITE_OK)
        throw std::runtime_error("sqlite3_close failed: "s + sqlite3_errmsg(std::exchange(db_, nullptr)));
}

void database_t::exec(const std::string& query, query_callback_t cb, void *context)
{
    assert(db_);
    char *err = nullptr;
    DEBUG("Exec: " << query);
    if(sqlite3_exec(db_, query.c_str(), cb, context, &err) != SQLITE_OK) {
        auto what = "sqlite3_exec failed: "s + err;
        sqlite3_free(err);
        throw std::runtime_error(std::move(what));
    }
}
