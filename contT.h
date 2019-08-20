#pragma once

#include "monad.h"
#include "traits.h"
#include <type_traits>

struct ContT_tag {};

template <typename E>
struct ContT {

    template <typename T>
    auto operator()(T k) const {
        return static_cast<const E&>(*this)(k);
    }

    operator E&() { return static_cast<E&>(*this); }
    operator E const&() const { return static_cast<const E&>(*this); }
};

template <typename F, typename A>
struct contT_impl;

template <typename F, typename A>
struct functor_traits<contT_impl<F, A>> {
    using tag = ContT_tag;
    using type = A;
};

template <typename F, typename A>
struct contT_impl : public ContT<contT_impl<F, A>> {
    contT_impl(F f) : f_(f) {}

    template <typename T>
    auto operator()(T k) const {
        return f_(k);
    }
    F f_;
};

template <typename F, typename A>
contT_impl<F, A> contT(F f) {
    return contT_impl<F, A>(f);
}

template <typename A>
struct Return_contT;

template <typename A>
struct functor_traits<Return_contT<A>> {
    using tag = ContT_tag;
    using type = A;
};

template <typename A>
struct Return_contT : public ContT<Return_contT<A>> {

    Return_contT(const A& x) : x_(x) {}

    template <typename T>
    auto operator()(T k) const {
        return k(x_);
    }

    A x_;
};

template <typename A>
Return_contT<A> return_contT(const A& x) {
    return Return_contT<A>(x);
}

// evalContT :: (Monad m) => ContT r m r -> m r
// evalContT (lift m) = m
template <typename M, typename C>
auto evalContT(C m) {
    // return m([](const auto& a) { return return_impl<M, decltype(a)>(a); });
    return m([](const auto& a) { return Return<M>(a); });
}

// return :: a -> ContT r m a
template <typename A>
struct return_impl<ContT_tag, A> : public ContT<return_impl<ContT_tag, A>> {

    return_impl(const A& x) : x_(x) {}

    template <typename T>
    auto operator()(T k) const {
        return k(x_);
    }

    A x_;
};

// (>>=) :: Cont r m a -> (a -> Cont r m b) -> Cont r m b
template <typename C, typename F>
struct bind_impl<C, F, typename std::enable_if<std::is_base_of<ContT<C>, C>::value>::type>
    : public ContT<bind_impl<C, F>> {
    bind_impl(C m, F k) : m_(m), k_(k) {}

    template <typename T>
    auto operator()(T f) const {
        return m_([this, f](const auto& a) { return k_(a)(f); });
    }

    C m_;
    F k_;
};

// callCC :: ((a -> ContT r m b) -> ContT r m a) -> ContT r m a
// callCC f = ContT $ \ c -> runContT (f (\ x -> ContT $ \ _ -> c x)) c

// (>>) :: Cont r a -> Cont r b -> Cont r b
template <typename A, typename B>
struct lbind_impl<
    A,
    B,
    typename std::enable_if<std::is_base_of<ContT<A>, A>::value && std::is_base_of<ContT<B>, B>::value>::type>
    : public ContT<lbind_impl<A, B>> {
    lbind_impl(A m1, B m2) : m1_(m1), m2_(m2) {}

    template <typename T>
    auto operator()(T f) const {
        return m1_([this, f](const auto&) { return m2_(f); });
    }

    A m1_;
    B m2_;
};

