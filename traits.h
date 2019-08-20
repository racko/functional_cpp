#pragma once

template <typename F, typename Enable = void>
struct functor_traits;

template <typename E>
typename functor_traits<E>::erased_type erase(E e) {
    return functor_traits<E>::erase(e);
}
