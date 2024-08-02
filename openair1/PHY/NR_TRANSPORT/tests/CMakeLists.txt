add_executable(test_llr test_llr.cpp)
target_link_libraries(test_llr PRIVATE PHY_NR GTest::gtest minimal_lib)
add_dependencies(tests test_llr)
add_test(NAME test_llr
  COMMAND ./test_llr)
