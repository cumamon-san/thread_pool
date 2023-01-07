#ifndef DATABASE_H
#define DATABASE_H

#include <string>

struct sqlite3;

class database_t
{
public:
    database_t();
    explicit database_t(const std::string& path);
    ~database_t();

    database_t(const database_t&) = delete;
    database_t& operator=(const database_t&) = delete;

    void open(const std::string& path);
    void close();

    using query_callback_t = int (*)(void*,int,char**,char**);
    void exec(const std::string& query, query_callback_t cb = nullptr, void *context = nullptr);

private:
    sqlite3 *db_;
};

#endif // DATABASE_H
