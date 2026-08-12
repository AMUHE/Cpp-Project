add_library(saw_vision face_recognizer.cpp)
add_library(SAW::Vision ALIAS saw_vision)
target_include_directories(saw_vision PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_vision PUBLIC Qt${QT_VERSION_MAJOR}::Core ${OpenCV_LIBS})
