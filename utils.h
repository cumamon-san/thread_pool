#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>
#include <functional>
#include <memory>
#include <utility>

namespace utils {
struct noncopyable_t {
  noncopyable_t() = default;
  noncopyable_t(noncopyable_t &&) = default;
  noncopyable_t &operator=(noncopyable_t &&) = default;
  noncopyable_t(const noncopyable_t &) = delete;
  noncopyable_t &operator=(const noncopyable_t &) = delete;
};

class scoped_fd_t {
public:
  explicit scoped_fd_t(int fd = -1) noexcept : fd_(fd) {}
  scoped_fd_t(scoped_fd_t &&other) noexcept : fd_(other.release()) {}
  scoped_fd_t &operator=(scoped_fd_t &&other) noexcept {
    if (this != &other)
      fd_ = other.release();
    return *this;
  }
  ~scoped_fd_t() noexcept { close(); }

  void close() noexcept {
    if (fd_ >= 0)
      ::close(fd_);
  };
  void reset(int fd = -1) noexcept {
    close();
    fd_ = fd;
  };
  int release() noexcept { return std::exchange(fd_, -1); }

  operator bool() { return fd_ >= 0; };
  operator int() { return fd_; };

private:
  int fd_;
};

class scoped_exit_t : noncopyable_t {
public:
  using handler_t = std::function<void()>;
  scoped_exit_t(handler_t&& handler) : handler_(std::move(handler)) {}
  ~scoped_exit_t() { handler_(); }
  handler_t release() { return std::exchange(handler_, {}); }

private:
  handler_t handler_;
};

} // namespace utils

#endif // UTILS_H
