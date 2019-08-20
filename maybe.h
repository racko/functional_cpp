#pragma once

#include "traits.h"
#include "monad.h"
#include <boost/optional.hpp>
#include <type_traits>

using boost::optional;

struct maybe_tag {};

template <typename E>
struct maybe {
    using type = typename functor_traits<E>::type;

    const type& value() const { return static_cast<const E&>(*this).value(); }

    type& value() { return static_cast<E&>(*this).value(); }

    explicit operator bool() const { return static_cast<const E&>(*this).operator bool(); }

    const type& operator*() const { return *static_cast<const E&>(*this); }

    type& operator*() { return *static_cast<E&>(*this); }

    operator E&() { return static_cast<E&>(*this); }
    operator E const&() const { return static_cast<const E&>(*this); }
};

template <typename A>
struct maybe_impl;

template <typename A>
struct functor_traits<maybe_impl<A>> {
    using tag = maybe_tag;
    using type = A;
    using erased_type = maybe_impl<type>;
    static erased_type erase(maybe_impl<A> x) { return x; }
};

template <typename A>
struct maybe_impl : public maybe<maybe_impl<A>> {
    using type = A;
    maybe_impl() = default;
    maybe_impl(const A& x) : x_(x) {}

    const type& value() const { return x_.value(); }

    type& value() { return x_.value(); }

    explicit operator bool() const { return static_cast<bool>(x_); }

    const type& operator*() const { return *x_; }

    type& operator*() { return *x_; }

    optional<type> x_;
};

template <typename A>
maybe_impl<A> just(const A& x) {
    return maybe_impl<A>(x);
}

template <typename A>
maybe_impl<A> nothing() {
    return maybe_impl<A>();
}

// struct static_nothing : public maybe<static_nothing> { };

template <typename A>
struct functor_traits<return_impl<maybe_tag, A>> {
    using tag = maybe_tag;
    using type = A;
};

template <typename A>
struct return_impl<maybe_tag, A> : public maybe<return_impl<maybe_tag, A>> {
    using type = A;
    return_impl(const type& x) : x_(x) {}

    const type& value() const { return x_; }

    type& value() { return x_; }

    explicit operator bool() const { return true; }

    const type& operator*() const { return x_; }

    type& operator*() { return x_; }

    type x_;
};

template <typename A>
struct functor_traits<fail_impl<maybe_tag, A>> {
    using tag = maybe_tag;
    using type = A;
};

template <typename A>
struct fail_impl<maybe_tag, A> : public maybe<fail_impl<maybe_tag, A>> {
    using type = A;
    fail_impl(const char*) {}

    const type& value() const { throw boost::bad_optional_access(); }

    type& value() { throw boost::bad_optional_access(); }

    explicit operator bool() const { return false; }

    const type& operator*() const { throw boost::bad_optional_access(); }

    type& operator*() { throw boost::bad_optional_access(); }
};

template <typename E, typename F>
struct functor_traits<bind_impl<E, F, typename std::enable_if<std::is_base_of<maybe<E>, E>::value>::type>> {
    using tag = maybe_tag;
    using type = typename std::result_of<F(typename E::type)>::type;
};

template <typename E, typename F>
struct bind_impl<E, F, typename std::enable_if<std::is_base_of<maybe<E>, E>::value>::type>
    : public maybe<bind_impl<E, F>> {
    using type = typename functor_traits<typename std::result_of<F(typename functor_traits<E>::type)>::type>::type;
    bind_impl(E m, F k) {
        if (!m)
            return;

        auto m2 = k(*m); // I would actually like to store the result of k(*m) in the bind_impl instead, but that would
                         // require default constructors

        if (m2)
            x_.reset(*m2);
    }

    const type& value() const { return x_.value(); }

    type& value() { return x_.value(); }

    explicit operator bool() const { return static_cast<bool>(x_); }

    const type& operator*() const { return *x_; }

    type& operator*() { return *x_; }

    optional<type> x_;
};

template <typename E1, typename E2>
struct functor_traits<lbind_impl<
    E1,
    E2,
    typename std::enable_if<std::is_base_of<maybe<E1>, E1>::value && std::is_base_of<maybe<E2>, E2>::value>::type>> {
    using tag = maybe_tag;
    using type = typename functor_traits<E2>::type;
};

template <typename E1, typename E2>
struct lbind_impl<
    E1,
    E2,
    typename std::enable_if<std::is_base_of<maybe<E1>, E1>::value && std::is_base_of<maybe<E2>, E2>::value>::type>
    : public maybe<lbind_impl<E1, E2>> {
    using type = typename E2::type;
    lbind_impl(E1 m1, E2 m2) {
        if (m1)
            x_.reset(*m2);
        else {
            x_.reset();
        }
    }

    const type& value() const { return x_.value(); }

    type& value() { return x_.value(); }

    explicit operator bool() const { return static_cast<bool>(x_); }

    const type& operator*() const { return *x_; }

    type& operator*() { return *x_; }

    optional<type> x_;
};
