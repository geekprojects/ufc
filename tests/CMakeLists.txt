
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/73a63ea05dc8ca29ec1d2c1d66481dd0de1950f1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

add_executable(ufc-data-tests
        data/route.cpp
        data/lnmprocedures.cpp
)
target_link_libraries(ufc-data-tests GTest::gtest_main ufc ufc-data)

gtest_discover_tests(ufc-data-tests)
