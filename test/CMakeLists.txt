cmake_minimum_required(VERSION 3.5.0)

include(CTest)
enable_testing()

add_executable(gs_arena_test gs/util/arena.c)
target_link_libraries(gs_arena_test PUBLIC gs)
target_include_directories(gs_arena_test PUBLIC include)

add_executable(gs_list_test gs/util/list.c)
target_link_libraries(gs_list_test PUBLIC gs)
target_include_directories(gs_list_test PUBLIC include)

add_executable(gs_string_test gs/util/string.c)
target_link_libraries(gs_string_test PUBLIC gs)
target_include_directories(gs_string_test PUBLIC include)

add_executable(gs_wstring_test gs/util/wstring.c)
target_link_libraries(gs_wstring_test PUBLIC gs)
target_include_directories(gs_wstring_test PUBLIC include)

add_executable(gs_buffer_test gs/util/buffer.c)
target_link_libraries(gs_buffer_test PUBLIC gs)
target_include_directories(gs_buffer_test PUBLIC include)

add_test(NAME gs_arena_test COMMAND $<TARGET_FILE:gs_arena_test>)
add_test(NAME gs_list_test COMMAND $<TARGET_FILE:gs_list_test>)
add_test(NAME gs_string_test COMMAND $<TARGET_FILE:gs_string_test>)
add_test(NAME gs_wstring_test COMMAND $<TARGET_FILE:gs_wstring_test>)
add_test(NAME gs_buffer_test COMMAND $<TARGET_FILE:gs_buffer_test>)