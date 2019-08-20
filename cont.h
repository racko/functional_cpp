#pragma once

#include "applicative.h"
#include "functor.h"
#include "monad.h"
#include "traits.h"
#include <type_traits>

struct Cont_tag {};

template <typename E>
struct Cont {

    template <typename T>
    auto operator()(T k) const {
        return static_cast<const E&>(*this)(k);
    }

    operator E&() { return static_cast<E&>(*this); }
    operator E const&() const { return static_cast<const E&>(*this); }
};

template <typename F, typename A>
struct cont_impl;

template <typename F, typename A>
struct functor_traits<cont_impl<F, A>> {
    using tag = Cont_tag;
    using type = A;
};

template <typename F, typename A>
struct cont_impl : public Cont<cont_impl<F, A>> {
    cont_impl(F f) : f_(f) {}

    template <typename T>
    auto operator()(T k) const {
        return f_(k);
    }
    F f_;
};

template <typename A, typename F>
cont_impl<F, A> cont(F f) {
    return cont_impl<F, A>(f);
}

template <typename A>
struct Return_cont;

template <typename A>
struct functor_traits<Return_cont<A>> {
    using tag = Cont_tag;
    using type = A;
};

template <typename A>
struct Return_cont : public Cont<Return_cont<A>> {

    Return_cont(const A& x) : x_(x) {}

    template <typename T>
    auto operator()(T k) const {
        return k(x_);
    }

    A x_;
};

template <typename A>
Return_cont<A> return_cont(const A& x) {
    return Return_cont<A>(x);
}

// evalCont :: Cont r r -> r
// evalCont (return x) = x
template <typename C, typename Enable = typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type>
auto evalCont(C m) {
    return m([](const auto& x) { return x; });
}

template <typename C, typename F>
struct MapCont;

template <typename C, typename F>
struct functor_traits<MapCont<C, F>> {
    using tag = Cont_tag;
    using type = typename functor_traits<C>::type;
};

// mapCont :: (r -> r) -> Cont r a -> Cont r a
// runCont (mapCont f m) = f . runCont m
// template<typename R, typename A>
// Cont<R,A> mapCont(function<R,R> f, Cont<R,A> m) {
template <typename C, typename F>
struct MapCont : public Cont<MapCont<C, F>> {

    MapCont(F f, C m) : f_(f), m_(m) {}

    template <typename T>
    auto operator()(T k) const {
        return f_(m_(k));
    }

    F f_;
    C m_;
};

template <typename C, typename F, typename Enable = typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type>
MapCont<C, F> mapCont(F f, C m) {
    return MapCont<C, F>(f, m);
}

template <typename C, typename F>
struct WithCont;

template <typename C, typename F>
struct functor_traits<WithCont<C, F>> {
    using tag = Cont_tag;
    // using type = typename std::result_of<F(typename functor_traits<C>::type)>::type;
};

// withCont :: ((b -> r) -> (a -> r)) -> Cont r a -> Cont r b
// runCont (withCont f m) = runCont m . f
// template<typename R, typename A, typename B>
// Cont<R,B> withCont(function<function<R,B>,function<R,A>> f, Cont<R,A> m) {
template <typename C, typename F>
struct WithCont : public Cont<WithCont<C, F>> {

    WithCont(F f, C m) : f_(f), m_(m) {}

    template <typename T>
    auto operator()(T k) const {
        return m_(f_(k));
    }

    F f_;
    C m_;
};

template <typename C, typename F, typename Enable = typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type>
WithCont<C, F> withCont(F f, C m) {
    return WithCont<C, F>(f, m);
}

template <typename A>
struct functor_traits<return_impl<Cont_tag, A>> {
    using tag = Cont_tag;
    using type = A;
    using erased_type = Return_cont<A>;
    static erased_type erase(return_impl<Cont_tag, A> x) { return return_cont(evalCont(x)); }
};

// return :: a -> Cont r a
template <typename A>
struct return_impl<Cont_tag, A> : public Cont<return_impl<Cont_tag, A>> {

    return_impl(const A& x) : x_(x) {}

    template <typename T>
    auto operator()(T k) const {
        return k(x_);
    }

    A x_;
};

template <typename C, typename F>
struct functor_traits<bind_impl<C, F>, typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type> {
    using tag = Cont_tag;
    using type = typename functor_traits<typename std::result_of<F(typename functor_traits<C>::type)>::type>::type;
    using erased_type = Return_cont<type>;
    static erased_type erase(bind_impl<C, F> x) { return return_cont(evalCont(x)); }
};

// (>>=) :: Cont r a -> (a -> Cont r b) -> Cont r b
template <typename C, typename F>
struct bind_impl<C, F, typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type>
    : public Cont<bind_impl<C, F>> {
    bind_impl(C m, F k) : m_(m), k_(k) {}

    template <typename T>
    auto operator()(T f) const {
        return m_([this, f](const auto& a) { return k_(a)(f); });
    }

    C m_;
    F k_;
};

// TODO: functor_traits<lbind_imp<...>>

// (>>) :: Cont r a -> Cont r b -> Cont r b
template <typename A, typename B>
struct lbind_impl<
    A,
    B,
    typename std::enable_if<std::is_base_of<Cont<A>, A>::value && std::is_base_of<Cont<B>, B>::value>::type>
    : public Cont<lbind_impl<A, B>> {
    lbind_impl(A m1, B m2) : m1_(m1), m2_(m2) {}

    template <typename T>
    auto operator()(T f) const {
        return m1_([this, f](const auto&) { return m2_(f); });
    }

    A m1_;
    B m2_;
};

template <typename C, typename F>
struct functor_traits<fmap_impl<C, F>, typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type> {
    using tag = Cont_tag;
    using type = typename std::result_of<F(typename functor_traits<C>::type)>::type;
};

// fmap :: (a -> b) -> Cont r a -> Cont r b
// fmap id      = id
// fmap (p . q) = (fmap p) . (fmap q)
template <typename C, typename F>
struct fmap_impl<C, F, typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type>
    : public Cont<fmap_impl<C, F>> {

    fmap_impl(F f, C c) : f_(f), c_(c) {}

    template <typename T>
    auto operator()(T k) const {
        return c_(compose(k, f_));
    }

    F f_;
    C c_;
};

template <typename C, typename A>
struct functor_traits<lfmap_impl<C, A>, typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type> {
    using tag = Cont_tag;
    using type = A;
};

// lfmap :: a -> Cont r b -> Cont r a
//   lfmap
// = fmap . const
// = (\f g x . f (g x)) fmap (\a . (\b . a))
// = \x . fmap ((\a . (\b . a)) x)
// = \x . fmap (\b . x)
// = \x . (\f m . (\k . m (k . f))) (\b . x)
// = \x . fmap (\b . x)
// = \x m . (\k . m (k . (\b . x)))
// = \x m . (\k . m ((\f g y . f (g y)) k (\b . x)))
// = \x m . (\k . m ((\y . k ((\b . x) y))))
// = \x m . (\k . m (\y . k x))
// e.g. lfmap :: a -> [b] -> [a] is lfmap x = map (const x)
template <typename C, typename A>
struct lfmap_impl<C, A, typename std::enable_if<std::is_base_of<Cont<C>, C>::value>::type>
    : public Cont<lfmap_impl<C, A>> {
    lfmap_impl(const A& a, C m) : a_(a), m_(m) {}

    template <typename T>
    auto operator()(T k) const {
        return m_([k, this](const auto&) { return k(a_); });
    }

    A a_;
    C m_;
};

////////////////////////////////////////////////////////////////////

// const id = (\a b . a)(\a . a) = \b a . a = flip const

// ap :: Cont r (a -> b) -> Cont r a -> Cont r b
// identity
//    pure id <*> v = v
// composition
//    pure (.) <*> u <*> v <*> w = u <*> (v <*> w)
// homomorphism
//    pure f <*> pure x = pure (f x)
// interchange
//    u <*> pure y = pure ($ y) <*> u
// The other methods have the following default definitions, which may be overridden with equivalent specialized
// implementations:
//    u *> v = pure (const id) <*> u <*> v
//    u <* v = pure const <*> u <*> v
// As a consequence of these laws, the Functor instance for f will satisfy
//    fmap f x = pure f <*> x
// If f is also a Monad, it should satisfy
//    pure = return
//    (<*>) = ap
//(which implies that pure and <*> satisfy the applicative functor laws).
template <typename F1, typename F2>
struct ap_impl<
    F1,
    F2,
    typename std::enable_if<std::is_base_of<Cont<F1>, F1>::value && std::is_base_of<Cont<F2>, F2>::value>::type>
    : public Cont<ap_impl<F1, F2>> {

    ap_impl(F1 f, F2 v) : f_(f), v_(v) {}

    template <typename T>
    auto operator()(T k) const {
        return f_([this, k](auto g) { return v_(compose(k, g)); });
    }

    F1 f_;
    F2 v_;
};

// lap :: Cont r a -> Cont r b -> Cont r a
// Sequence actions, discarding the value of the second argument.
// template<typename R, typename A, typename B>
// Cont<R,A> lap(Cont<R,A> u, Cont<R,B> v) {
//    return [u,v] (function<R,A> k) { return u([v,k] (const A& a) { return v([k,a] (const B& b) { return k(a); }); });
//    };
//}
template <typename F1, typename F2>
struct lap_impl<
    F1,
    F2,
    typename std::enable_if<std::is_base_of<Cont<F1>, F1>::value && std::is_base_of<Cont<F2>, F2>::value>::type>
    : public Cont<lap_impl<F1, F2>> {

    lap_impl(F1 u, F2 v) : u_(u), v_(v) {}

    template <typename T>
    auto operator()(T k) const {
        return u_([this, k](const auto& a) { return v_([k, a](const auto&) { return k(a); }); });
    }

    F1 u_;
    F2 v_;
};

// rap :: Cont r a -> Cont r b -> Cont r b
// Sequence actions, discarding the value of the first argument.
template <typename F1, typename F2>
struct rap_impl<
    F1,
    F2,
    typename std::enable_if<std::is_base_of<Cont<F1>, F1>::value && std::is_base_of<Cont<F2>, F2>::value>::type>
    : public Cont<rap_impl<F1, F2>> {

    rap_impl(F1 u, F2 v) : u_{u}, v_{v} {}

    template <typename T>
    auto operator()(T k) const {
        return u_([this, k](const auto& a) { return v_([k, a](const auto& b) { return k(b); }); });
    }

    F1 u_;
    F2 v_;
};
