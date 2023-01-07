#include <filesystem>
#include <vector>
#include <string>

#include "database.h"
#include "log.h"

namespace fs = std::filesystem;

int main() {
    std::vector<fs::path> paths;
    for(auto&& item : fs::recursive_directory_iterator(".")) {
        if(fs::is_regular_file(item))
            paths.push_back(item);
    }

    database_t db("index.db");
    for(auto&& path : paths) {
        db.insert(path, fs::file_size(path));
    }

    db.print();

    return 0;
}
