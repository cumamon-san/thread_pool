#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include "database.h"
#include "log.h"

using namespace std::string_literals;

static int select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
    auto counter = reinterpret_cast<int*>(p_data);
    DEBUG_VAR(num_fields);
    for(int i = 0; i < num_fields; ++i)
        DEBUG("Item[" << *counter << "]: " << p_col_names[i] << " = " << p_fields[i]);
    ++(*counter);
    return 0;
}

int main() {
    database_t db("index.db");

    srand(time(nullptr));
    db.exec("CREATE TABLE IF NOT EXISTS foo(PID, RANDOM);"
            "INSERT INTO FOO VALUES(" + std::to_string(getpid()) + ", " + std::to_string(rand()) + ");");

    int counter = 0;
    db.exec("SELECT * FROM FOO;", select_callback, &counter);

    DEBUG("Done");
    return 0;
}
