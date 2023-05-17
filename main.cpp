#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>

#include "spdlog/spdlog.h"

#include "utils.h"

#include <functional>
#include <type_traits>

#define DEBUG_EXPR(X) spdlog::debug(#X " = {}", (X))

// template <typename Res, typename... Args>
// class func_holder_t : utils::noncopyable_t {
//   class data_holder_t {
//   public:
//     static constexpr size_t size = sizeof(void *);

//     void *raw_data() { return data_; }
//     const void *raw_data() const { return data_; }
//     template <typename T> T &get_as() {
//       static_assert(sizeof(T) <= size);
//       return &reinterpret_cast<T *>(raw_data());
//     }
//     template <typename T> const T &get_as() const {
//       static_assert(sizeof(T) <= size);
//       return &static_cast<const T *>(raw_data());
//     }

//   private:
//     std::uint8_t data_[size];
//   };

//   template <typename Functor> class base {
//     data_holder_t data_;

//     static constexpr bool stored_locally =
//         sizeof(Functor) <= data_holder_t::size;

//     Functor *get_pointer(const data_holder_t &dh) {
//       if constexpr (stored_locally) {
//         const Functor &f = data_.template raw_data<Functor>();
//         return const_cast<Functor *>(std::addressof(f));
//       } else // have stored a pointer
//         return data_.template raw_data<Functor *>();
//     }

//     static void create(data_holder_t &dh, Functor &&f, std::true_type) {
//       new (dh.raw_data()) Functor(std::forward<Functor>(f));
//     }

//     // Construct a function object on the heap and store a pointer.
//     static void create(data_holder_t &dh, Functor &&f, std::false_type) {
//       dh.template get_as<Functor *>() = new
//       Functor(std::forward<Functor>(f));
//     }

//     // Destroy an object stored in the internal buffer.
//     static void destroy(data_holder_t &dh, std::true_type) {
//       dh.template get_as<Functor>().~Functor();
//     }

//     // Destroy an object located on the heap.
//     static void destroy(data_holder_t &dh, std::false_type) {
//       delete dh.template get_as<Functor *>();
//     }

//     explicit base(Functor &&f) {
//       create(data_, std::forward<Functor>(f), stored_locally);
//     }

//     static auto invoke(const data_holder_t &dh, Args &&...args) {
//       return std::invoke(*get_pointer(dh), std::forward<Args>(args)...);
//     }
//   };

// public:
//   template <typename Fn> static void test(Fn &&Call) {
//     base(std::forward<Fn>(Call)).invoke();
//   }
// };
template <typename Signature> class func_holder_t;

template <typename Res, typename... Args> struct func_holder_t<Res(Args...)> {
  struct interface_t {
    virtual Res invoke(Args &&...) = 0;
    virtual ~interface_t() {}
  };
  template <typename Func> struct impl_t : interface_t {
    impl_t(Func &&f) : func(std::forward<Func>(f)) {}
    Res invoke(Args &&...args) override {
      return std::invoke(func, std::forward<Args>(args)...);
    }
    Func func;
  };

  template <typename Func>
  func_holder_t(Func &&f)
      : impl(std::make_unique<impl_t<Func>>(std::forward<Func>(f))) {}

  Res invoke(Args &&...args) {
    return impl->invoke(std::forward<Args>(args)...);
  }

  std::unique_ptr<interface_t> impl;
};

int print() {
  spdlog::warn("From print");
  return 0;
}

int main() {
  spdlog::set_pattern("[%^%l%$] %v");
  spdlog::set_level(spdlog::level::debug);

  // DEBUG_EXPR("From main");

  {
    func_holder_t<int()> h(print);
    h.invoke();
  }

  {
    func_holder_t<int()> h([] {
      spdlog::warn("From lambda");
      return 5;
    });
    h.invoke();
  }

  return EXIT_SUCCESS;
}
