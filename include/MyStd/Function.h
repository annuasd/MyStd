// Function核心知识点
// 通过多态+虚函数, 将返回值和参数类型相同的可调用对象擦除为同一个虚类进行存储

#pragma once
#include <memory>
#include <stdexcept>
#include <utility>
namespace mystd {

template <class Fn> class Function;

template <class Ret, class... Args> class Function<Ret(Args...)> {
  struct CallableBase {
    virtual Ret operator()(Args... args) = 0;
    virtual ~CallableBase() {}
  };
  template <class F> struct Callable : CallableBase {
    F f;
    template<class U>
    Callable(U &&f) : f(f) {}
    Ret operator()(Args... args) override {
      return f(std::forward<Args>(args)...);
    }
  };
  std::unique_ptr<CallableBase> callable;

public:
  Function() = default;
  template <class F>
  Function(F &&f) : callable(std::make_unique<Callable<F>>(f)) {}
  Ret operator()(Args... args) {
    if (!callable.get())
      throw std::runtime_error("Function not initialized");
    return callable->operator()(std::forward<Args>(args)...);
  }
};

} // namespace mystd