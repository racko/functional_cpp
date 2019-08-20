#include "contT.h"

#include "maybe.h"
#include "monad.h"
#include <catch2/catch.hpp>

TEST_CASE("testEvalContT", "") {
    auto result = evalContT<maybe_tag>(return_contT(42));
    REQUIRE(result.value() == 42);
}

TEST_CASE("testReturnContT", "") {
    auto result = Return<ContT_tag>(42);
    REQUIRE(evalContT<maybe_tag>(result).value() == 42);
}

TEST_CASE("testBindContT", "") {
    auto result1 = return_contT(42) >>= [](int x) { return return_contT(2 * x); };
    REQUIRE(evalContT<maybe_tag>(result1).value() == 84);

    auto result3 = return_contT(23) >> return_contT(42) >> return_contT(1);
    REQUIRE(evalContT<maybe_tag>(result3).value() == 1);
}
