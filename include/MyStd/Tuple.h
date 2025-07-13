// 核心知识点:
// 空基类优化。通过判断一个基类为空基类，用继承而不是作为变量的方式存储该类
// 这里注意一下为什么NodeStorage要加index作为参数列表
// 比如 calss Empty{}; template<class T0, class T1> class Derived : T0, T1;
// 如果 Derived<Empty, Empty>即 模板参数 T0 和 T1相等，则无法采用空基类优化
// 所以我们把index放到参数列表中，将他们区分开

#pragma once
#include <cstddef>
#include <type_traits>
namespace mystd {

template <size_t, class, class = void> class NodeStorage;
template <size_t index, class T>
class NodeStorage<index, T,
                  std::enable_if_t<std::is_empty_v<T> && !std::is_final_v<T>>>
    : private T {
  static constexpr bool falgs = true;

protected:
  NodeStorage() = default;
  template <class U> NodeStorage(U &&u) : T(u) {}
  T &get() { return *this; }
  T const &get() const { return *this; }
};

template <size_t index, class T>
class NodeStorage<index, T, std::enable_if_t<!std::is_empty_v<T>>> {
  T t;

protected:
  NodeStorage() = default;
  NodeStorage(T t) : t(t) {}
  T &get() { return t; }
  T const &get() const { return t; }
};

template <size_t index, class... Ts> class TupleImpl;

template <size_t index> class TupleImpl<index> {};
template <size_t index, class T0, class... Ts>
class TupleImpl<index, T0, Ts...> : protected NodeStorage<index, T0>,
                                    protected TupleImpl<index + 1, Ts...> {
protected:
  template <class U, class... Us>
  TupleImpl(U t0, Us... ts)
      : NodeStorage<index, T0>(std::forward<U>(t0)),
        TupleImpl<index + 1, Ts...>(std::forward<Us>(ts)...) {}
  TupleImpl() = default;
  template <size_t idx> decltype(auto) get() {
    if constexpr (idx == index) {
      return NodeStorage<index, T0>::get();
    } else {
      return TupleImpl<index + 1, Ts...>::template get<idx>();
    }
  }
  template <size_t idx> decltype(auto) get() const {
    if constexpr (idx == index) {
      return NodeStorage<index, T0>::get();
    } else {
      return TupleImpl<index + 1, Ts...>::template get<idx>();
    }
  }
};

template <class... Ts> class Tuple : protected TupleImpl<0, Ts...> {
public:
  Tuple() = default;
  template <class... Us>
  Tuple(Us... ts) : TupleImpl<0, Ts...>(std::forward<Us>(ts)...) {}
  template <size_t index> decltype(auto) get() {
    return TupleImpl<0, Ts...>::template get<index>();
  }
  template <size_t index> decltype(auto) get() const {
    return TupleImpl<0, Ts...>::template get<index>();
  }
  static constexpr size_t size() { return sizeof...(Ts); }
};
}; // namespace mystd