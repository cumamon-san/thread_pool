#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include "utils.h"

struct sqlite3;

class database_t : utils::noncopyable_t {
public:
    explicit database_t(const std::string& path);
    ~database_t();

    void insert(const std::string& path, size_t size);
    void print() const;
    void print_all() const;

    struct error;
    class stmt;

private:
    database_t() : db_(nullptr) {}
    void open(const std::string& path);
    void close();
    using query_callback_t = int (*)(void*,int,char**,char**);
    void exec(const std::string& query, query_callback_t cb = nullptr, void *context = nullptr);

    sqlite3 *db_;
};

#endif // DATABASE_H
