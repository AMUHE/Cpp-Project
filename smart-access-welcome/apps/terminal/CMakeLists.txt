add_executable(saw_terminal WIN32 main.cpp main_window.cpp main_window.h main_window.ui)
target_link_libraries(saw_terminal PRIVATE
    SAW::Camera SAW::Vision SAW::AccessServer SAW::AccessControl SAW::Config
    SAW::Persistence SAW::Device SAW::Speech
    SAW::Logging
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
)
set_target_properties(saw_terminal PROPERTIES OUTPUT_NAME "SmartAccessWelcome")

find_file(SAW_FACE_CASCADE haarcascade_frontalface_default.xml
    HINTS
        "${OpenCV_DIR}/../../../etc/haarcascades"
        "${OpenCV_DIR}/../../etc/haarcascades"
        "${OpenCV_DIR}/../etc/haarcascades")
if(SAW_FACE_CASCADE)
    add_custom_command(TARGET saw_terminal POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SAW_FACE_CASCADE}" "$<TARGET_FILE_DIR:saw_terminal>"
        VERBATIM)
else()
    message(WARNING "haarcascade_frontalface_default.xml was not found; set SAW_CASCADE_PATH at runtime")
endif()

install(TARGETS saw_terminal RUNTIME DESTINATION bin)
install(FILES ${PROJECT_SOURCE_DIR}/config/config.example.json DESTINATION config)
if(SAW_FACE_CASCADE)
    install(FILES "${SAW_FACE_CASCADE}" DESTINATION bin)
endif()
