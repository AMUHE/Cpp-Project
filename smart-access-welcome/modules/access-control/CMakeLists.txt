add_library(saw_access_control access_policy.cpp)
add_library(SAW::AccessControl ALIAS saw_access_control)
target_include_directories(saw_access_control PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
