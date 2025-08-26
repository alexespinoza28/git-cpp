# Google Test setup
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(
  gitcpp_tests
  test_basic_operations.cpp
  test_branching.cpp
  test_merging.cpp
)

target_link_libraries(
  gitcpp_tests
  gtest_main
)

# Link against the main gitcpp source files
target_include_directories(gitcpp_tests PRIVATE ../src)
target_sources(gitcpp_tests PRIVATE
  ../src/Repository.cpp
  ../src/Utils.cpp
  ../src/Commands.cpp
  ../src/Commit.cpp
)

include(GoogleTest)
gtest_discover_tests(gitcpp_tests)