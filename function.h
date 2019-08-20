#pragma once

template <typename F, typename G>
struct Composition {
    Composition(F f, G g) : f_(f), g_(g) {}

    template <typename T>
    auto operator()(const T& a) const {
        return f_(g_(a));
    }
    F f_;
    G g_;
};

template <typename F, typename G>
Composition<F, G> compose(F f, G g) {
    return Composition<F, G>(f, g);
}

