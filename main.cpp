#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>

#include "sqlite3.h"

#define ERROR(X) do { std::cerr << "ERROR: " << __func__ << ": " << X << std::endl; } while(0)
#define PRINT(X) do { std::cout << X << std::endl; } while(0)
#define DEBUG(X) do { std::cout << "DEBUG: " << X << std::endl; } while(0)
#define DEBUG_VAR(X) DEBUG(#X " = " << X)

static int select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
    auto counter = reinterpret_cast<int*>(p_data);
    DEBUG_VAR(num_fields);
    for(int i = 0; i < num_fields; ++i)
        DEBUG("Item[" << *counter << "]: " << p_col_names[i] << " = " << p_fields[i]);
    ++(*counter);
    return 0;
}

int main(int argc, char **argv) {
    std::string db_path("my_database.sqlite3");
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db(nullptr, sqlite3_close);

    DEBUG("Try to open " << db_path);
    if(sqlite3 *db_ptr; sqlite3_open(db_path.c_str(), &db_ptr) != SQLITE_OK) {
        ERROR("sqlite3_open failed: " << sqlite3_errmsg(db.get()));
        exit(EXIT_FAILURE);
    }
    else {
        db.reset(db_ptr);
    }

    srand(time(nullptr));
    std::string sql = "CREATE TABLE IF NOT EXISTS foo(PID, RANDOM);"
                      "INSERT INTO FOO VALUES(" + std::to_string(getpid()) + ", " + std::to_string(rand()) + ");";
    DEBUG("Exec: " << sql);
    if(char *err = nullptr; sqlite3_exec(db.get(), sql.c_str(), 0, 0, &err) != SQLITE_OK) {
        ERROR("sqlite3_exec failed: " << err);
        sqlite3_free(err);
    }

    sql = "SELECT * FROM FOO;";
    int counter = 0;
    DEBUG("Exec: " << sql);
    if(char *err = nullptr; sqlite3_exec(db.get(), sql.c_str(), select_callback, &counter, &err) != SQLITE_OK) {
        ERROR("sqlite3_exec failed: " << err);
        sqlite3_free(err);
    }

    DEBUG("Done");
    return 0;
}
