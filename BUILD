cc_library(
    name = "test_main",
    srcs = [
        "test_main.cpp",
    ],
    linkstatic = True,
)

cc_library(
    name = "functional",
    hdrs = [
        "applicative.h",
        "cont.h",
        "contT.h",
        "function.h",
        "functor.h",
        "maybe.h",
        "maybeT.h",
        "monad.h",
        "traits.h",
    ],
)

cc_test(
    name = "functional_test",
    srcs = [
        "contT_test.cpp",
        "cont_test.cpp",
        "maybeT_test.cpp",
        "maybe_test.cpp",
    ],
    deps = [
        ":functional",
        ":test_main",
    ],
)
