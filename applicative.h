#pragma once

template <typename F1, typename F2, typename Enable = void>
struct ap_impl;

// const id = (\a b . a)(\a . a) = \b a . a = flip const

// ap :: f (a -> b) -> f a -> f b
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
ap_impl<F1, F2> ap(F1 f, F2 v) {
    return ap_impl<F1, F2>(f, v);
}

template <typename F1, typename F2, typename Enable = void>
struct lap_impl;

// lap :: f a -> f b -> f a
// Sequence actions, discarding the value of the second argument.
template <typename F1, typename F2>
lap_impl<F1, F2> lap(F1 u, F2 v) {
    return lap_impl<F1, F2>(u, v);
}

template <typename F1, typename F2, typename Enable = void>
struct rap_impl;

// rap :: f a -> f b -> f b
// Sequence actions, discarding the value of the first argument.
template <typename F1, typename F2>
rap_impl<F1, F2> rap(F1 u, F2 v) {
    return rap_impl<F1, F2>(u, v);
}
