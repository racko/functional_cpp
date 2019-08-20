#pragma once

#include "function.h"
#include "traits.h"
#include <type_traits>

template <typename M, typename A>
struct return_impl;

template <typename M, typename A>
struct fail_impl;

template <typename M, typename F, typename Enable = void>
struct bind_impl;

template <typename M1, typename M2, typename Enable = void>
struct lbind_impl;

// return :: a -> m a
template <typename M, typename A>
return_impl<M, A> Return(const A& x) {
    return return_impl<M, A>(x);
}

template <typename M, typename A>
fail_impl<M, A> fail(const char* error_msg) {
    return fail_impl<M, A>(error_msg);
}

// FIXME: wrong associativity
// Should be left-associative, is right-associative
// Also, its precedence should be the same as >>, but it's lower.
// Also, it since it is an unconstrained template, it will match all types, not just the ones in the haskell type
// (>>=) :: m a -> (a -> m b) -> m b
template <typename M, typename F>
bind_impl<M, F> operator>>=(M m, F k) {
    return bind_impl<M, F>(m, k);
}

// (>>) :: m a -> m b -> m b
template <typename M1, typename M2>
lbind_impl<M1, M2> operator>>(M1 m1, M2 m2) {
    return lbind_impl<M1, M2>(m1, m2);
}

template <typename T, typename M, typename Enable = void>
struct lift_impl;

// lift :: Monad m => m a -> t m a
template <typename T, typename M>
lift_impl<T, M> lift(M a) {
    return lift_impl<T, M>(a);
}

// liftM :: Monad m => (a1 -> r) -> m a1 -> m r
template <typename F, typename M>
auto liftM(F f, M m1) {
    return m1 >>= compose(Return<typename functor_traits<M>::tag,
                                 typename std::result_of<F(typename functor_traits<M>::type)>::type>,
                          f);
    // return m1 >>= [f] (const auto& x1) { return return_impl<typename functor_traits<M>::tag,typename
    // std::result_of<F(decltype(x1))>::type>(f(x1)); };
}
