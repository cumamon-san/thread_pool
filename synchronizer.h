#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <cassert>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "utils.h"

namespace utils {

template <typename Data> class synchronizer_t : noncopyable_t {
public:
  explicit synchronizer_t(Data *ptr = new Data{}) : ptr_(ptr) { assert(ptr_); }

  template <typename Type, template<typename> typename Lock>
  class accessor : utils::noncopyable_t {
  public:
    accessor(Type *value, std::shared_mutex &mtx)
      : value_(value), lock_(mtx) {}
    Type *operator->() { return value_; }
    Type &operator*() { return *value_; }

  private:
    Type *value_;
    Lock<std::shared_mutex> lock_;
  };

  auto unique() { return accessor<Data, std::unique_lock>(ptr_.get(), mtx_); }
  auto shared() const { return accessor<const Data, std::shared_lock>(ptr_.get(), mtx_); }

private:
  std::unique_ptr<Data> ptr_;
  mutable std::shared_mutex mtx_;
};

} // namespace utils

#endif // SYNCHRONIZER_H
