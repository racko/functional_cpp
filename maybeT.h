#pragma once

#include "maybe.h"
#include "monad.h"
#include "traits.h"
#include <type_traits>

struct maybeT_tag {};

template <typename M>
struct maybeT_tag_ {};

template <typename E>
struct maybeT {
    using type = typename functor_traits<E>::type;

    type operator()() const { return static_cast<const E&>(*this)(); }

    operator E&() { return static_cast<E&>(*this); }
    operator E const&() const { return static_cast<const E&>(*this); }
};

template <typename M>
struct maybeT_impl;

template <typename M>
struct functor_traits<maybeT_impl<M>> {
    using type = typename functor_traits<M>::type;
    using tag = maybeT_tag_<typename functor_traits<M>::tag>;
};

template <typename M>
struct maybeT_impl : public maybeT<maybeT_impl<M>> {
    maybeT_impl(const M& x) : x_(x) {}

    M operator()() const { return x_; }
    M x_;
};

template <typename M>
struct functor_traits<lift_impl<maybeT_tag, M>> {
    using type = typename functor_traits<M>::type;
    using tag = maybeT_tag_<typename functor_traits<M>::tag>;
};

// liftM :: (a1 -> r) -> m a1 -> m r
// liftM f m1 = m1 >>= \x1 -> return (f x1)
//
// MaybeT :: m (Maybe a) -> MaybeT m a
// Just :: a -> Maybe a
//
// lift :: m a -> t m a
// lift m = MaybeT (liftM Just m) = m >>= \x1 -> return (Just x1)
template <typename M>
struct lift_impl<maybeT_tag, M> : public maybeT<lift_impl<maybeT_tag, M>> {
    lift_impl(M a) : a_(a) {}

    auto operator()() const { return liftM(just<typename functor_traits<M>::type>, a_); }

    M a_;
};

template <typename M, typename A>
struct functor_traits<return_impl<maybeT_tag_<M>, A>> {
    using type = A;
    using tag = maybeT_tag_<M>;
};

template <typename M, typename A>
struct return_impl<maybeT_tag_<M>, A> : public maybeT<return_impl<maybeT_tag_<M>, A>> {
    using type = A;
    return_impl(const type& x) : x_(x) {}

    auto operator()() const { return lift<maybeT_tag>(Return<M>(x_))(); }

    type x_;
};

template <typename M, typename A>
struct functor_traits<fail_impl<maybeT_tag_<M>, A>> {
    using tag = maybeT_tag_<M>;
    using type = A;
};

template <typename M, typename A>
struct fail_impl<maybeT_tag_<M>, A> : public maybeT<fail_impl<maybeT_tag_<M>, A>> {
    using type = A;
    fail_impl(const char*) {}

    auto operator()() const { return Return<M>(nothing<type>()); }
};

template <typename E, typename F>
struct functor_traits<bind_impl<E, F, typename std::enable_if<std::is_base_of<maybeT<E>, E>::value>::type>> {
    using type = typename functor_traits<typename std::result_of<F(typename functor_traits<E>::type)>::type>::type;
    using tag = typename functor_traits<E>::tag;
};

template <typename M>
struct maybeT_inner;

template <typename M>
struct maybeT_inner<maybeT_tag_<M>> {
    using type = M;
};

template <typename E, typename F>
struct bind_impl<E, F, typename std::enable_if<std::is_base_of<maybeT<E>, E>::value>::type>
    : public maybeT<bind_impl<E, F>> {
    using type = typename functor_traits<typename std::result_of<F(typename functor_traits<E>::type)>::type>::type;
    using inner = typename maybeT_inner<typename functor_traits<E>::tag>::type;

    bind_impl(E m, F k) : m_(m), k_(k) {}

    auto operator()() const {
        return m_() >>= [this](const auto& v) {
            if (v) {
                return erase(k_(*v)());
            } else {
                return erase(Return<inner, maybe_impl<type>>(nothing<type>()));
            }
        };
    }

    E m_;
    F k_;
};

//TODO

//template <typename E1, typename E2>
//struct functor_traits<lbind_impl<
//    E1,
//    E2,
//    typename std::enable_if<std::is_base_of<maybeT<E1>, E1>::value && std::is_base_of<maybeT<E2>, E2>::value>::type>> {
//    using tag = maybeT_tag;
//    using type = typename functor_traits<E2>::type;
//};
//
//template <typename E1, typename E2>
//struct lbind_impl<
//    E1,
//    E2,
//    typename std::enable_if<std::is_base_of<maybeT<E1>, E1>::value && std::is_base_of<maybeT<E2>, E2>::value>::type>
//    : public maybeT<lbind_impl<E1, E2>> {
//    using type = typename E2::type;
//    lbind_impl(E1 m1, E2 m2) {
//        if (m1)
//            x_.reset(*m2);
//        else {
//            x_.reset();
//        }
//    }
//
//    const type& value() const { return x_.value(); }
//
//    type& value() { return x_.value(); }
//
//    explicit operator bool() const { return static_cast<bool>(x_); }
//
//    const type& operator*() const { return *x_; }
//
//    type& operator*() { return *x_; }
//
//    optional<type> x_;
//};
