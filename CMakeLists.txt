cmake_minimum_required (VERSION 2.8.11)
project (j4backlight)

find_library(M_LIB m)


add_executable (j4backlight j4backlight.c)
target_link_libraries(j4backlight ${M_LIB})
