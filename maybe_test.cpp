#include "maybe.h"

#include <catch2/catch.hpp>
#include <type_traits>
#include "monad.h"

TEST_CASE("maybe_impl is a maybe", "") {
    REQUIRE(std::is_base_of<maybe<maybe_impl<int>>, maybe_impl<int>>::value);
}

TEST_CASE("testReturnMaybe", "") {
    auto result = Return<maybe_tag>(42);
    REQUIRE(result.value() == 42);
}

TEST_CASE("testFailMaybe", "") {
    auto result = fail<maybe_tag,int>("Error!");
    REQUIRE(!result);
}

TEST_CASE("testBindMaybe", "") {
    auto result1 = just(42) >>= [](int x){ return just(2 * x); };
    REQUIRE(result1.value() == 84);
    auto result2 = nothing<int>() >>= [](int x){ return just(2 * x); };
    REQUIRE(!result2);

    auto result3 = just(23) >> just(42);
    REQUIRE(result3.value() == 42);
    auto result4 = nothing<int>() >> just(42);
    REQUIRE(!result4);
}
