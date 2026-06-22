add_library(get_input get_input.cpp)
target_include_directories(get_input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(get_input lib_log)
target_link_libraries(get_input error_code)