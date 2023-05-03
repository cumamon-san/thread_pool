#include "database.h"

#include <cassert>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "sqlite3.h"

#include "spdlog/spdlog.h"

#define TABLE_NAME "files"

using namespace std::string_literals;
namespace fs = std::filesystem;

struct database_t::error_t : std::runtime_error {
  explicit error_t(const std::string &msg) : std::runtime_error(msg) {}
  error_t(const std::string &from, sqlite3 *db)
      : std::runtime_error(from + ": " + sqlite3_errmsg(db)) {}
};

class database_t::stmt_t : utils::noncopyable_t {
public:
  stmt_t() = default;
  stmt_t(sqlite3 *db, const char *query) {
    if (sqlite3_prepare_v2(db, query, -1, &ptr_, NULL) != SQLITE_OK)
      throw error_t("sqlite3_prepare_v2", db);
  }
  sqlite3_stmt *get() const { return ptr_; }
  ~stmt_t() { sqlite3_finalize(ptr_); }

private:
  sqlite3_stmt *ptr_ = nullptr;
};

class database_t::transaction_t : utils::noncopyable_t {
public:
  explicit transaction_t(database_t& db) : db_(db), commied_(false) {}

  void begin() {
    spdlog::debug("BEGIN TRANSACTION");
    db_.exec("BEGIN TRANSACTION");
    commied_ = false;
  }

  void commit() {
    spdlog::debug("COMMIT TRANSACTION");
    db_.exec("COMMIT TRANSACTION");
    commied_ = true;
  }

  ~transaction_t() { if (!commied_) db_.exec("ROLLBACK"); }

private:
  database_t& db_;
  bool commied_;
};

database_t::database_t(const std::string &path) : database_t() {
  open(path);
  exec("DROP TABLE IF EXISTS " TABLE_NAME);
  exec("CREATE TABLE IF NOT EXISTS " TABLE_NAME R"~((
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        path TEXT NOT NULL UNIQUE,
        size INTEGER NOT NULL)
    )~");
}

void database_t::insert(const file_info_t& info) {
  static const char query[] =
      "INSERT INTO " TABLE_NAME " (path, size) VALUES(?,?) "
      "ON CONFLICT(path) DO UPDATE SET size=?";
  stmt_t stmt(db_, query);
  sqlite3_bind_text(stmt.get(), 1, info.path.c_str(), -1, 0);
  sqlite3_bind_int(stmt.get(), 2, info.size);
  sqlite3_bind_int(stmt.get(), 3, info.size);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE)
    throw error_t("sqlite3_step", db_);
}

void database_t::insert_many(const std::vector<file_info_t>& infos, int transaction_size) {
  transaction_t tras(*this);
  tras.begin();
  int counter = 0;
  for (auto&& info : infos) {
    insert(info);
    if (++counter == transaction_size) {
      spdlog::debug("Insert {} items", counter);
      tras.commit();
      tras.begin();
      counter = 0;
    }
  }
  spdlog::debug("Insert {} items", counter);
  tras.commit();
}


void database_t::print() const {
  stmt_t stmt(db_, "select * from " TABLE_NAME);
  spdlog::debug("Got results:");
  while (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    fs::path path(reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 1)));
    spdlog::debug("File {} has size: {} bytes", path.filename().c_str(), sqlite3_column_int(stmt.get(), 2));
  }
}

void database_t::print_all() const {
  stmt_t stmt(db_, "select * from " TABLE_NAME);
  spdlog::debug("Got results:");
  while (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    std::ostringstream os;
    int num_cols = sqlite3_column_count(stmt.get());
    for (int i = 0; i < num_cols; ++i) {
      if (os.tellp())
        os << ", ";
      switch (sqlite3_column_type(stmt.get(), i)) {
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
    spdlog::debug(os.str());
  }
}

database_t::~database_t() {
  if (db_)
    close();
}

void database_t::open(const std::string &path) {
  if (db_)
    close();

  spdlog::debug("Open {}", path);
  if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
    throw error_t("sqlite3_open", std::exchange(db_, nullptr));
}

void database_t::close() {
  spdlog::debug("Close database");
  if (sqlite3_close(db_) != SQLITE_OK)
    throw error_t("sqlite3_close", std::exchange(db_, nullptr));
}

void database_t::exec(const std::string &query, query_callback_t cb,
                      void *context) {
  assert(db_);
  char *err = nullptr;
  //    spdlog::debug("Exec: {}", query);
  if (sqlite3_exec(db_, query.c_str(), cb, context, &err) != SQLITE_OK) {
    error_t sql_err("sqlite3_exec: "s + err);
    sqlite3_free(err);
    throw sql_err;
  }
}
