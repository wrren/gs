cmake_minimum_required(VERSION 3.5.0)
project(gs VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

file(GLOB_RECURSE gs_SOURCES CONFIGURE_DEPENDS "src/gs/*.c")
file(GLOB_RECURSE gs_HEADERS CONFIGURE_DEPENDS "src/gs/*.h")

set(gs_cli_SOURCES src/cli/main.c)

add_library(gs STATIC ${gs_SOURCES})
target_link_libraries(gs shlwapi.lib ntdll.lib)
target_include_directories(gs PUBLIC include)
target_compile_definitions(gs PUBLIC -DUNICODE -D_UNICODE)

add_executable(gs_cli ${gs_cli_SOURCES})
target_link_libraries(gs_cli gs)
target_include_directories(gs_cli PUBLIC include)
target_compile_definitions(gs_cli PUBLIC -DUNICODE -D_UNICODE)

add_subdirectory(test)