#pragma once

template <typename F, typename M, typename Enable = void>
struct fmap_impl;

// fmap :: (a -> b) -> f a -> f b
// fmap id      = id
// fmap (p . q) = (fmap p) . (fmap q)
template <typename Functor, typename Morphism>
fmap_impl<Functor, Morphism> fmap(Morphism f, Functor m) {
    return fmap_impl<Functor, Morphism>(f, m);
}

template <typename F, typename A, typename Enable = void>
struct lfmap_impl;

// lfmap :: a -> f b -> f a
// lfmap = fmap . const
template <typename F, typename A>
lfmap_impl<F, A> lfmap(const A& a, F m) {
    return lfmap_impl<F, A>(a, m);
}
