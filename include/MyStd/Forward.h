#include <type_traits>
namespace mystd{
    // Forward 就是把一个Value的Type重新转为Category
    template<class T>
    T&& forward(typename std::remove_reference<T>::type& t) {
        return static_cast<T&&>(t);
    }
    template<class T>
    T&& forward(typename std::remove_reference<T>::type&& t) {
        return static_cast<T&&>(t);
    }
}
