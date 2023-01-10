#ifndef UTILS_H
#define UTILS_H

namespace utils {
    struct noncopyable_t {
        noncopyable_t() = default;
        noncopyable_t(const noncopyable_t&) = delete;
        noncopyable_t& operator = (const noncopyable_t&) = delete;
    };
}

#endif // UTILS_H

