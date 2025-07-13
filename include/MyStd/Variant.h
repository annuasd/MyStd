// Variant核心知识点

// 1.
//  通过递归模板，获得 可变模板参数包中模板T的index，以及通过index获得模板参数
// 
// 2. 
// 通过sfinae/折叠表达式 识别对象是否相同或可构造
//
// 3.
// 静态分发。通过函数指针数组+边长表达式静态生产函数表，实现在运行时拿到动态index可以分发给对应类型的函数效果
// 这里核心在于 类型为静态信息，而index为动态信息 -> 查表解决

#pragma once
#include <algorithm>
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>
namespace mystd {
template <class, class, class = void> struct VariantIndex;
template <class, class, class = void> struct VariantConvertibleIndex;
template <class, size_t> struct VariantAlternative;

class BadVariantAccess : public std::exception {
public:
  virtual const char *what() const noexcept { return "bad variant access"; }
};

template <class Fn, class... Ts> struct IsVisitorHasSingleRetType;

template <class Fn, class T0> struct IsVisitorHasSingleRetType<Fn, T0> {
  static constexpr bool value = true;
};

template <class Fn, class T0, class T1, class... Ts>
struct IsVisitorHasSingleRetType<Fn, T0, T1, Ts...> {
  static constexpr bool value = std::is_same_v<std::invoke_result_t<Fn, T0>,
                                               std::invoke_result_t<Fn, T1>> &&
                                IsVisitorHasSingleRetType<Fn, T1, Ts...>::value;
};
template <class T0, class... Ts> struct FisrtType {
  using type = T0;
};

template <class... Ts> struct Variant {
public:
  template <class T, class = std::enable_if_t<
                         std::disjunction_v<std::is_convertible<T, Ts>...>>>
  Variant(T t) : index(VariantConvertibleIndex<Variant, T>::value) {
    using RealType = typename VariantAlternative<
        Variant, VariantConvertibleIndex<Variant, T>::value>::type;
    auto *storagePtr = reinterpret_cast<RealType *>(storage);
    new (storagePtr) RealType(t);
  }
  Variant(Variant const &other) {
    index = other.index;
    copyConstructFns[index](other.storage, storage);
  }
  Variant(Variant &&other) {
    index = other.index;
    moveConstructFns[index](other.storage, storage);
  }
  Variant &operator=(Variant const &other) {
    index = other.index;
    copyAssignFns[index](other.storage, storage);
    return *this;
  }
  Variant &operator=(Variant &&other) {
    index = other.index;
    moveAssignFns[index](other.storage, storage);
    return *this;
  }

  template <class T,
            class = std::enable_if<std::disjunction_v<std::is_same<T, Ts>...>>>
  T &get() {
    if (index != VariantIndex<Variant, T>::value)
      throw BadVariantAccess();
    return *reinterpret_cast<T *>(storage);
  }

  ~Variant() { destructFns[index](storage); }

  template <class Fn> decltype(auto) accept(Fn &&visitor) {
    static_assert((std::is_invocable_v<Fn, Ts> && ...),
                  "`accept` requires the visitor to be exhaustive.");
    static_assert(
        IsVisitorHasSingleRetType<Fn, Ts...>::value,
        "`accept` requires the visitor to have a single return type.");
    using RetType = std::invoke_result_t<Fn, typename FisrtType<Ts...>::type>;

    using VisitFnType = RetType (*)(char *, Fn);
    VisitFnType visitorTbl[sizeof...(Ts)] = {
        [](char *storage, Fn visitor) -> RetType {
          auto *t = reinterpret_cast<Ts *>(storage);
          return visitor(*t);
        }...};
    return visitorTbl[index](storage, std::forward<Fn>(visitor));
  }

private:
  using CopyFn = void (*)(char const *, char *);
  using MoveFn = void (*)(char *, char *);
  CopyFn copyConstructFns[sizeof...(Ts)] = {[](char const *from, char *to) {
    auto *t = reinterpret_cast<Ts const *>(from);
    new (to) Ts(*t);
  }...};
  MoveFn moveConstructFns[sizeof...(Ts)] = {[](char *from, char *to) {
    auto *t = reinterpret_cast<Ts *>(from);
    new (to) Ts(std::move(*t));
  }...};
  CopyFn copyAssignFns[sizeof...(Ts)] = {[](char const *from, char *to) {
    auto *t = reinterpret_cast<Ts const *>(from);
    auto *other = reinterpret_cast<Ts *>(to);
    (*other) = (*t);
  }...};
  MoveFn moveAssignFns[sizeof...(Ts)] = {[](char *from, char *to) {
    auto *t = reinterpret_cast<Ts *>(from);
    auto *other = reinterpret_cast<Ts *>(to);
    (*other) = std::move(*t);
  }...};

  using DestructFn = void (*)(char *);

  DestructFn destructFns[sizeof...(Ts)] = {[](char *storage) {
    auto *t = reinterpret_cast<Ts *>(storage);
    t->~Ts();
  }...};

  size_t index;
  alignas(std::max({sizeof(Ts)...})) char storage[std::max({sizeof(Ts)...})];
};

template <class T, class... Ts>
using is_one_of_t =
    std::enable_if_t<std::disjunction_v<std::is_same<T, Ts>...>>;

template <class T, class... Ts>
using is_one_of_convertible_t =
    std::enable_if_t<!std::disjunction_v<std::is_same<T, Ts>...> &&
                     std::disjunction_v<std::is_convertible<T, Ts>...>>;

template <class T0, class T, class... Ts>
struct VariantIndex<Variant<T0, Ts...>, T,
                    std::enable_if_t<std::is_same_v<T, T0>>> {
  constexpr static size_t value = 0;
};

template <class T0, class T, class... Ts>
struct VariantIndex<Variant<T0, Ts...>, T, is_one_of_t<T, Ts...>> {
  constexpr static size_t value = VariantIndex<Variant<Ts...>, T>::value + 1;
};

template <class T0, class T, class... Ts>
struct VariantConvertibleIndex<
    Variant<T0, Ts...>, T,
    std::enable_if_t<!std::is_same_v<T, T0> && !std::is_convertible_v<T, T0> &&
                     !std::disjunction_v<std::is_same<T, Ts>...> &&
                     std::disjunction_v<std::is_convertible<T, Ts>...>>> {
  constexpr static size_t value =
      VariantConvertibleIndex<Variant<Ts...>, T>::value + 1;
};

template <class T0, class T, class... Ts>
struct VariantConvertibleIndex<Variant<T0, Ts...>, T,
                               is_one_of_t<T, T0, Ts...>> {
  constexpr static size_t value = VariantIndex<Variant<T0, Ts...>, T>::value;
};

template <class T0, class T, class... Ts>
struct VariantConvertibleIndex<
    Variant<T0, Ts...>, T,
    std::enable_if_t<
        std::is_convertible_v<T, T0> &&
        !std::disjunction_v<std::is_same<T, T0>, std::is_same<T, Ts>...>>> {
  constexpr static size_t value = 0;
};

template <class T, class... Ts>
struct VariantAlternative<Variant<T, Ts...>, 0> {
  using type = T;
};

template <class T, size_t I, class... Ts>
struct VariantAlternative<Variant<T, Ts...>, I> {
  using type = typename VariantAlternative<Variant<Ts...>, I - 1>::type;
};

} // namespace mystd