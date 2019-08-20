#include "maybeT.h"

#include "cont.h"
#include "monad.h"
#include <catch2/catch.hpp>

TEST_CASE("testReturnMaybeT", "") {
    auto result = Return<maybeT_tag_<Cont_tag>>(42);
    REQUIRE(evalCont(result()).value() == 42);
}

TEST_CASE("testFailMaybeT", "") {
    auto result = fail<maybeT_tag_<Cont_tag>, int>("Error!");
    REQUIRE(!evalCont(result()));
}

TEST_CASE("testBindMaybeT", "") {
    auto result1 = Return<maybeT_tag_<Cont_tag>>(42) >>= [](int x) { return Return<maybeT_tag_<Cont_tag>>(2 * x); };
    REQUIRE(evalCont(result1()).value() == 84);
    auto result2 = fail<maybeT_tag_<Cont_tag>, int>("") >>= [](int x) { return Return<maybeT_tag_<Cont_tag>>(2 * x); };
    REQUIRE(!evalCont(result2()));

    // TODO
    //auto result3 = Return<maybeT_tag_<Cont_tag>>(23) >> Return<maybeT_tag_<Cont_tag>>(42);
    //REQUIRE(evalCont(result3()).value() == 42);
    //auto result4 = fail<maybeT_tag_<Cont_tag>, int>("") >> Return<maybeT_tag_<Cont_tag>>(42);
    //REQUIRE(!evalCont(result4()));
}
