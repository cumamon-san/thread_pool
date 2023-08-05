#ifndef MOVE_ONLY_FUNCTION_H
#define MOVE_ONLY_FUNCTION_H

#include <type_traits>

namespace utils {

namespace detail {

class undefined_class;

union locally_stored_types {
  void *void_ptr;
  void (*func_ptr)();
  void (undefined_class::*method_ptr)();
};

class data_storage_t {
public:
  static constexpr size_t size = sizeof(locally_stored_types);

  void *get_raw() { return reinterpret_cast<void *>(buff.data()); }
  const void *get_raw() const {
    return reinterpret_cast<const void *>(buff.data());
  }

  template <typename T>
    requires(sizeof(T) <= size)
  T &get_as() {
    return *reinterpret_cast<T *>(buff.data());
  }
  template <typename T>
    requires(sizeof(T) <= size)
  const T &get_as() const {
    return *reinterpret_cast<const T *>(buff.data());
  }

private:
  std::array<std::byte, size> buff;
};

enum manager_operation_t {
  move_functor,
  destroy_functor,
  move_and_destroy_functor
};

struct base_function_t {
  template <typename Functor> class base_manager_t {
  protected:
    static constexpr bool stored_locally =
        sizeof(Functor) <= data_storage_t::size;

    // Retrieve a pointer to the function object
    static Functor *get_pointer(const data_storage_t &src) noexcept {
      if constexpr (stored_locally) {
        const Functor &f = src.template get_as<Functor>();
        return const_cast<Functor *>(std::addressof(f));
      } else // have stored a pointer
        return src.template get_as<Functor *>();
    }

  private:
    template <typename Fn> static void create(data_storage_t &storage, Fn &&f) {
      if constexpr (stored_locally)
        new (storage.get_raw()) Functor(std::forward<Fn>(f));
      else
        storage.template get_as<Functor *>() = new Functor(std::forward<Fn>(f));
    }

    static void move(data_storage_t &dst, data_storage_t &src) {
      auto &src_ref = *get_pointer(src);
      create(dst, std::move(src_ref));
    }

    static void destroy(data_storage_t &storage) {
      if constexpr (stored_locally)
        storage.template get_as<Functor>().~Functor();
      else
        delete storage.template get_as<Functor *>();
    }

  public:
    static void manager(data_storage_t &dst, data_storage_t &src,
                        manager_operation_t op) {
      switch (op) {
      case move_functor:
        move(dst, src);
        break;
      case destroy_functor:
        destroy(dst);
        break;
      case move_and_destroy_functor:
        move(dst, src);
        destroy(src);
        break;
      }
    }

    template <typename Fn>
    static void init_functor(data_storage_t &storage, Fn &&f) {
      create(storage, std::forward<Fn>(f));
    }
  };

  base_function_t() = default;

  ~base_function_t() {
    if (manager)
      manager(functor, functor, destroy_functor);
  }

protected:
  using manager_t = void (*)(data_storage_t &, data_storage_t &,
                             manager_operation_t);

  data_storage_t functor;
  manager_t manager = nullptr;
};

template <typename Signature, typename Functor> class function_handler_t;

template <typename Res, typename Functor, typename... Args>
class function_handler_t<Res(Args...), Functor>
    : public base_function_t::base_manager_t<Functor> {
  using base = base_function_t::base_manager_t<Functor>;

public:
  using base::manager;
  static Res invoke(const data_storage_t &functor, Args &&...args) {
    return std::invoke(*base::get_pointer(functor),
                       std::forward<Args>(args)...);
  }
};

} // namespace detail

template <typename Signature> class move_only_function_t;

template <typename Res, typename... Args>
class move_only_function_t<Res(Args...)> : detail::base_function_t {

public:
  template <typename Functor>
    requires std::is_invocable_r_v<Res, Functor, Args...>
  move_only_function_t(Functor &&f) {
    using handler_t =
        detail::function_handler_t<Res(Args...), std::decay_t<Functor>>;

    handler_t::init_functor(functor, std::forward<Functor>(f));
    invoker = &handler_t::invoke;
    manager = &handler_t::manager;
  }

  Res operator()(Args... args) const {
    if (*this)
      return invoker(functor, std::forward<Args>(args)...);
    throw std::bad_function_call();
  }

  void swap(move_only_function_t &other) {
    std::swap(invoker, other.invoker);

    detail::data_storage_t tmp_functor;
    manager_t tmp_manager = nullptr;
    if (manager)
      manager(tmp_functor, functor, detail::move_and_destroy_functor);
    tmp_manager = manager;

    if (other.manager)
      other.manager(functor, other.functor, detail::move_and_destroy_functor);
    manager = other.manager;

    if (tmp_manager)
      tmp_manager(other.functor, tmp_functor, detail::move_and_destroy_functor);
    other.manager = tmp_manager;
  }

  move_only_function_t() = default;

  move_only_function_t(move_only_function_t &&other) : move_only_function_t() {
    swap(other);
  }

  move_only_function_t &operator=(move_only_function_t &&other) {
    if (&other != this) {
      reset();
      swap(other);
    }
    return *this;
  }

  template <typename Functor>
    requires std::is_invocable_r_v<Res, Functor, Args...>
  move_only_function_t &operator=(Functor &&f) {
    move_only_function_t(std::forward<Functor>(f)).swap(*this);
    return *this;
  }

  void reset() {
    if (manager) {
      manager(functor, functor, detail::destroy_functor);
      manager = nullptr;
      invoker = nullptr;
    }
  }

  inline move_only_function_t &operator=(std::nullptr_t) {
    reset();
    return *this;
  }

  inline operator bool() const { return manager; }

private:
  using invoker_t = Res (*)(const detail::data_storage_t &, Args &&...);
  invoker_t invoker = nullptr;
};

} // namespace utils

#endif // MOVE_ONLY_FUNCTION_H
