#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <memory>
#include <shared_mutex>

#include "utils.h"

template<typename Data>
class synchronizer_t : utils::noncopyable {
public:
    synchronizer_t() : ptr_(new Data) {}
    synchronizer_t(Data *ptr) : ptr_(ptr) {}

    template<typename T>
    class unique_access : utils::noncopyable {
    public:
        unique_access(T* value, std::shared_mutex* mtx) : value_(value), mtx_(mtx) { mtx_->lock(); }
        ~unique_access() { mtx_->unlock(); }
        T* operator -> () { return value_; }
        T& operator * () { return *value_; }
    private:
        T* value_;
        std::shared_mutex* mtx_;
    };

    template<typename T>
    class shared_access : utils::noncopyable {
    public:
        shared_access(T* value, std::shared_mutex* mtx) : value_(value), mtx_(mtx) { mtx_->lock_shared(); }
        ~shared_access() { mtx_->unlock_shared(); }
        T* operator -> () { return value_; }
        T& operator * () { return *value_; }
    private:
        T* value_;
        std::shared_mutex* mtx_;
    };

    auto unique() { return unique_access<Data>(ptr_.get(), &mtx_); }
    auto unique() const { return unique_access<const Data>(ptr_.get(), &mtx_); }

    auto shared() { return shared_access<Data>(ptr_.get(), &mtx_); }
    auto shared() const { return shared_access<const Data>(ptr_.get(), &mtx_); }

private:
    std::unique_ptr<Data> ptr_;
    mutable std::shared_mutex mtx_;
};

#endif // SYNCHRONIZER_H
