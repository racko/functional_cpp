#include "cont.h"

#include "functor.h"
#include "monad.h"
#include <catch2/catch.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

TEST_CASE("testEvalCont", "") { REQUIRE(evalCont(return_cont(42)) == 42); }

TEST_CASE("testFmapCont", "") {
    REQUIRE(2 ==
            evalCont(fmap([](const std::string& s) { return s.size(); },
                          fmap([](int x) { return std::to_string(x); }, cont<int>([](auto f) { return f(42); })))));
}

TEST_CASE("testReturnCont", "") {
    auto result = Return<Cont_tag>(42);
    REQUIRE(evalCont(result) == 42);
}

TEST_CASE("testBindCont", "") {
    auto result1 = return_cont(42) >>= [](int x) { return return_cont(2 * x); };
    REQUIRE(evalCont(result1) == 84);

    auto result3 = return_cont(23) >> return_cont(42) >> return_cont(1);
    REQUIRE(evalCont(result3) == 1);
}

template <typename B, typename A>
struct cast_impl;

template <typename B, typename A>
cast_impl<B, A> cast(const A& x) {
    return cast_impl<B, A>(x);
}

template <>
struct cast_impl<size_t, long long> : Cont<cast_impl<size_t, long long>> {

    cast_impl(const long long& x) : x_(x) {}

    template <typename T>
    auto operator()(T f) const {
        // if (x >= 0)
        return f(static_cast<size_t>(x_));
        // else
        //    return fail<R,size_t>("x < 0");
    }
    long long x_;
};

Return_cont<long long> fromString(const char* const& x) { return return_cont(std::atoll(x)); }

template <typename R>
cont_impl<std::function<R(std::function<R(size_t)>)>, int> factorial(size_t n) {
    // return cont([n] (auto f) { return n < 2 ? f(1) : factorial<R>(n-1).f([n,f] (size_t x) { return f(n*x); }); });
    return cont<int>(std::function<R(std::function<R(size_t)>)>(
        [n](auto f) { return n < 2 ? f(1) : factorial<R>(n - 1)([n, f](size_t x) { return f(n * x); }); }));
}

// template<typename R>
// Cont<R,int> factorial2(int n) {
//    return n < 0 ? fail<R,int>("n < 0") : [n] (function<R,int> f) { return n < 2 ? f(1) : factorial<R>(n-1)([n,f]
//    (size_t x) { return f(n*x); }); };
//}

template <typename R>
auto comp(const char* s) {
    auto a = return_cont(s);
    auto b = a >>= fromString;
    auto c = b >>= cast<size_t, long long>;
    auto d = c >>= factorial<R>;
    return d;
}

template auto comp<void>(const char*);

template <typename T>
void print(const T& x) {
    std::cout << x << '\n';
}

TEST_CASE("composition test 1", "") {
    const char* arg = "4";
    auto str = evalCont(return_cont(arg));
    auto n = evalCont(fromString(str));
    auto un = evalCont(cast<size_t>(n));        // "unsigned n"
    auto fun = evalCont(factorial<size_t>(un)); // "f(unsigned n)"

    REQUIRE(str == "4");
    REQUIRE(n == 4);
    REQUIRE(un == 4ULL);
    REQUIRE(fun == 24ULL);
}

TEST_CASE("composition test 2", "") { REQUIRE(evalCont(comp<std::size_t>("4")) == 24); }
