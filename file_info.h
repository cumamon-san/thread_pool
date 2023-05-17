#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <filesystem>

struct file_info_t {
    file_info_t(std::filesystem::path p, size_t s) : path(std::move(p)), size(s) {}
    std::filesystem::path path;
    size_t size;
};

#endif // FILE_INFO_H
