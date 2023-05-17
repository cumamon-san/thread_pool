#ifndef MOVE_ONLY_FUNCTION_H
#define MOVE_ONLY_FUNCTION_H

template <typename T> class move_only_function : public std::function<T> {
  template <typename Fn, typename En = void> struct wrapper;

  // specialization for CopyConstructible Fn
  template <typename Fn>
  struct wrapper<Fn, std::enable_if_t<std::is_copy_constructible<Fn>::value>> {
    template <typename... Args> auto operator()(Args &&...args) {
      return fn(std::forward<Args>(args)...);
    }

    Fn fn;
  };

  // specialization for MoveConstructible-only Fn
  template <typename Fn>
  struct wrapper<Fn, std::enable_if_t<!std::is_copy_constructible<Fn>::value &&
                                      std::is_move_constructible<Fn>::value>> {
    wrapper(Fn &&fn) : fn(std::forward<Fn>(fn)) {}

    wrapper(wrapper &&) = default;
    wrapper &operator=(wrapper &&) = default;

    wrapper(const wrapper &) = delete;
    wrapper &operator=(const wrapper &) = default;

    // // these two functions are instantiated by std::function
    // // and are never called
    // wrapper(const wrapper &rhs) : fn(const_cast<Fn &&>(rhs.fn)) {
    //   throw 0;
    // } // hack to initialize fn for non-DefaultContructible types
    // wrapper &operator=(wrapper &) { throw 0; }

    template <typename... Args> auto operator()(Args &&...args) {
      return fn(std::forward<Args>(args)...);
    }

    Fn fn;
  };

  using base = std::function<T>;

public:
  move_only_function() noexcept = default;
  move_only_function(std::nullptr_t) noexcept : base(nullptr) {}

  template <typename Fn>
  move_only_function(Fn &&f) : base(wrapper<Fn>{std::forward<Fn>(f)}) {}

  move_only_function(move_only_function &&) = default;
  move_only_function &operator=(move_only_function &&) = default;

  move_only_function &operator=(std::nullptr_t) {
    base::operator=(nullptr);
    return *this;
  }

  template <typename Fn> move_only_function &operator=(Fn &&f) {
    base::operator=(wrapper<Fn>{std::forward<Fn>(f)});
    return *this;
  }

  using base::operator();
};

#endif // MOVE_ONLY_FUNCTION_H
