#include "gtest/gtest.h"

#include <unordered_set>

#include "move_only_function.h"

static void inc_arg(int &counter) { ++counter; }

template <typename T> using funct_t = utils::move_only_function_t<T>;

TEST(MOFunction, Constructors) {
  using wrapper_t = funct_t<void(int &)>;

  {
    int counter = 0;

    wrapper_t{inc_arg}(counter);
    EXPECT_EQ(counter, 1);

    wrapper_t{&inc_arg}(counter);
    EXPECT_EQ(counter, 2);
  }

  {
    int counter = 0;

    wrapper_t wrapper;
    EXPECT_THROW(wrapper(counter), std::bad_function_call);

    wrapper = {};
    EXPECT_THROW(wrapper(counter), std::bad_function_call);

    wrapper = nullptr;
    EXPECT_THROW(wrapper(counter), std::bad_function_call);

    wrapper = inc_arg;
    wrapper(counter);
    EXPECT_EQ(counter, 1);

    wrapper = &inc_arg;
    wrapper(counter);
    EXPECT_EQ(counter, 2);
  }

  {
    wrapper_t w1([](int &c) { ++c; });
    wrapper_t w2 = std::move(w1);

    int counter = 0;
    EXPECT_THROW(w1(counter), std::bad_function_call);

    w2(counter);
    EXPECT_EQ(counter, 1);
  }
}

struct CheckDestructor {
  CheckDestructor() { EXPECT_TRUE(items.insert(this).second); }
  CheckDestructor(CheckDestructor &&) : CheckDestructor() {}

  CheckDestructor(const CheckDestructor &) = delete;
  CheckDestructor &operator=(const CheckDestructor &) = delete;
  CheckDestructor &operator=(CheckDestructor &&) = delete;

  ~CheckDestructor() {
    EXPECT_TRUE(items.contains(this));
    items.erase(this);
  }

  void operator()(int &c) const { ++c; }

  inline static std::unordered_set<CheckDestructor *> items;
};

TEST(MOFunction, Destructors) {
  using wrapper_t = utils::move_only_function_t<void(int &)>;
  {
    wrapper_t w1(CheckDestructor{});
    wrapper_t w2(CheckDestructor{});
    wrapper_t w3(CheckDestructor{});
    EXPECT_EQ(CheckDestructor::items.size(), 3UL);

    wrapper_t w4(std::move(w3));
    EXPECT_EQ(CheckDestructor::items.size(), 3UL);

    w2 = std::move(w4);
    EXPECT_EQ(CheckDestructor::items.size(), 2UL);

    w2 = nullptr;
    EXPECT_EQ(CheckDestructor::items.size(), 1UL);
  }
  EXPECT_EQ(CheckDestructor::items.size(), 0UL);
}

TEST(MOFunction, Methods) {
  int counter = 0;
  CheckDestructor cd;
  {
    using wrapper_t = funct_t<void(CheckDestructor *, int &)>;
    wrapper_t w(&CheckDestructor::operator());
    w(&cd, counter);
    EXPECT_EQ(counter, 1);
  }
  {
    using wrapper_t = funct_t<void(int &)>;
    wrapper_t w =
        std::bind(&CheckDestructor::operator(), &cd, std::placeholders::_1);
    w(counter);
    EXPECT_EQ(counter, 2);
  }
}
