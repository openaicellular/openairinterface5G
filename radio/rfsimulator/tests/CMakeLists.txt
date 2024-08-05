add_executable(test_rfsimulator test_rfsimulator.cpp)
target_link_libraries(test_rfsimulator PRIVATE rfsimulator_static GTest::gtest minimal_lib nr_common)
add_dependencies(tests test_rfsimulator)
add_test(NAME test_rfsimulator
  COMMAND ./test_rfsimulator)
