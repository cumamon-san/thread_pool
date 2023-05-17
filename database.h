#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

#include "file_info.h"
#include "utils.h"

struct sqlite3;

class database_t : utils::noncopyable_t {
public:
    explicit database_t(const std::string& path);
    ~database_t();

    void insert(const file_info_t& info);
    void insert_many(const std::vector<file_info_t>& infos, int transaction_size = 1000);
    void print() const;
    void print_all() const;

    struct error_t;
    class stmt_t;
    class transaction_t;

private:
    database_t() : db_(nullptr) {}
    void open(const std::string& path);
    void close();
    using query_callback_t = int (*)(void*,int,char**,char**);
    void exec(const std::string& query, query_callback_t cb = nullptr, void *context = nullptr);

    sqlite3 *db_;
};

#endif // DATABASE_H
