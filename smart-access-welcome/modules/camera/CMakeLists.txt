add_library(saw_camera stereo_camera.cpp)
add_library(SAW::Camera ALIAS saw_camera)
target_include_directories(saw_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_camera PUBLIC ${OpenCV_LIBS})
