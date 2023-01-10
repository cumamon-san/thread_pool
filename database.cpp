#include "database.h"

#include <cassert>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <sqlite3.h>

#include "log.h"

#define TABLE_NAME "files"

using namespace std::string_literals;
namespace fs = std::filesystem;

struct database_t::error : std::runtime_error {
    explicit error(const std::string& msg)
    : std::runtime_error(msg) {}
    error(const std::string& from, sqlite3 *db)
    : std::runtime_error(from + ": " + sqlite3_errmsg(db)) {}
};

class database_t::stmt : utils::noncopyable_t {
public:
    sqlite3_stmt *get() const { return ptr; }
    sqlite3_stmt **get_addr() { return &ptr; }
    ~stmt() { sqlite3_finalize(ptr); }

private:
    sqlite3_stmt *ptr = nullptr;
};


database_t::database_t(const std::string& path)
: database_t()
{
    open(path);
//    exec("DROP TABLE IF EXISTS " TABLE_NAME);
    exec("CREATE TABLE IF NOT EXISTS " TABLE_NAME R"~((
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        path TEXT NOT NULL UNIQUE,
        size INTEGER NOT NULL)
    )~");
}

void database_t::insert(const std::string& path, size_t size)
{
    DEBUG("Insert " << path);
    static const char query[] = "INSERT INTO " TABLE_NAME " (path, size) VALUES(?,?) "
                                "ON CONFLICT(path) DO UPDATE SET size=?";
    stmt stmt;
    if(sqlite3_prepare_v2(db_, query, -1, stmt.get_addr(), NULL) != SQLITE_OK)
        throw error("sqlite3_prepare_v2", db_);

    sqlite3_bind_text(stmt.get(), 1, path.c_str(), -1, 0);
    sqlite3_bind_int(stmt.get(), 2, size);
    sqlite3_bind_int(stmt.get(), 3, size);
    if(sqlite3_step(stmt.get()) != SQLITE_DONE)
        throw error("sqlite3_step", db_);
}

void database_t::print() const
{
    stmt stmt;
    if(sqlite3_prepare_v2(db_, "select * from " TABLE_NAME, -1, stmt.get_addr(), NULL) != SQLITE_OK)
        throw error("sqlite3_prepare_v2", db_);

    DEBUG("Got results:");
    while(sqlite3_step(stmt.get()) != SQLITE_DONE) {
        fs::path path(reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1)));
        DEBUG("File " << path.filename() << " has size: " << sqlite3_column_int(stmt.get(), 2) << " bytes");
    }
}

void database_t::print_all() const
{
    stmt stmt;
    if(sqlite3_prepare_v2(db_, "select * from " TABLE_NAME, -1, stmt.get_addr(), NULL) != SQLITE_OK)
        throw error("sqlite3_prepare_v2", db_);

    DEBUG("Got results:");
    while(sqlite3_step(stmt.get()) != SQLITE_DONE) {
        std::ostringstream os;
        int num_cols = sqlite3_column_count(stmt.get());
        for(int i = 0; i < num_cols; ++i) {
            if(os.tellp()) os << ", ";
            switch(sqlite3_column_type(stmt.get(), i)) {
                case (SQLITE3_TEXT):
                    os << sqlite3_column_text(stmt.get(), i);
                    break;
                case (SQLITE_INTEGER):
                    os << sqlite3_column_int(stmt.get(), i);
                    break;
                case (SQLITE_FLOAT):
                    os << sqlite3_column_double(stmt.get(), i);
                    break;
                default:
                    break;
            }
        }
        DEBUG(os.str());
    }
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
        throw error("sqlite3_open", std::exchange(db_, nullptr));
}

void database_t::close()
{
    DEBUG("Close database");
    if(sqlite3_close(db_) != SQLITE_OK)
        throw error("sqlite3_close", std::exchange(db_, nullptr));
}

void database_t::exec(const std::string& query, query_callback_t cb, void *context)
{
    assert(db_);
    char *err = nullptr;
//    DEBUG("Exec: " << query);
    if(sqlite3_exec(db_, query.c_str(), cb, context, &err) != SQLITE_OK) {
        error sql_err("sqlite3_exec: "s + err);
        sqlite3_free(err);
        throw sql_err;
    }
}
