#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <cassert>
#include <memory>
#include <shared_mutex>

#include "utils.h"

namespace utils {

template<typename Data>
class synchronizer_t : noncopyable_t {
public:
    explicit synchronizer_t(Data *ptr = new Data {}) : ptr_(ptr) { assert(ptr_); }

    template<typename T>
    class unique_access : utils::noncopyable_t {
    public:
        unique_access(T* value, std::shared_mutex& mtx) : value_(value), lock_(mtx) {}
        T* operator -> () { return value_; }
        T& operator * () { return *value_; }
    private:
        T* value_;
        std::unique_lock<std::shared_mutex> lock_;
    };

    template<typename T>
    class shared_access : utils::noncopyable_t {
    public:
        shared_access(T* value, std::shared_mutex& mtx) : value_(value), lock_(mtx) {}
        T* operator -> () { return value_; }
        T& operator * () { return *value_; }
    private:
        T* value_;
        std::shared_lock<std::shared_mutex> lock_;
    };

    auto unique() { return unique_access<Data>(ptr_.get(), mtx_); }
    auto unique() const { return unique_access<const Data>(ptr_.get(), mtx_); }

    auto shared() { return shared_access<Data>(ptr_.get(), mtx_); }
    auto shared() const { return shared_access<const Data>(ptr_.get(), mtx_); }

private:
    std::unique_ptr<Data> ptr_;
    mutable std::shared_mutex mtx_;
};

} // namespace utils

#endif // SYNCHRONIZER_H
