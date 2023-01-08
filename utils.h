#ifndef UTILS_H
#define UTILS_H

namespace utils {
    struct noncopyable {
        noncopyable() = default;
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator = (const noncopyable&) = delete;
    };
}

#endif // UTILS_H

