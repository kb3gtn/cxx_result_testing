add_executable( result_test result_test.cpp )
target_include_directories( result_test PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_compile_features(result_test PRIVATE cxx_std_17)
