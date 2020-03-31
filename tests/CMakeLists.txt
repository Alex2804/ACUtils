cmake_minimum_required(VERSION 2.8.12)
project(APluginSDKTest LANGUAGES C)

add_subdirectory(libs/check)

add_executable(APluginSDKTest src/test_dynarray.c)
target_link_libraries(APluginSDKTest check)
add_test(NAME APluginLibraryTest COMMAND APluginLibraryTest)
add_compile_definitions(APluginSDKTest PRIVATE VALGRIND_SUPPORTED)
