add_library(saw_device door_controller.cpp)
add_library(SAW::Device ALIAS saw_device)
target_include_directories(saw_device PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_device PUBLIC Qt${QT_VERSION_MAJOR}::Core)
