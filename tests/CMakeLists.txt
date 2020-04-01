cmake_minimum_required(VERSION 2.8.12)
project(CUtilsTest LANGUAGES C)

add_subdirectory(libs/check)

add_executable(CUtilsTest src/test_dynarray.c)
set_property(TARGET CUtilsTest PROPERTY C_STANDARD 90)
target_link_libraries(CUtilsTest check)
