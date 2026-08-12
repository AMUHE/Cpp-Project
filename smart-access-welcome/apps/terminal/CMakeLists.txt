add_executable(saw_terminal WIN32 main.cpp main_window.cpp main_window.h main_window.ui)
target_link_libraries(saw_terminal PRIVATE
    SAW::Camera SAW::Vision SAW::AccessServer SAW::AccessControl SAW::Config
    SAW::Persistence SAW::Device SAW::Speech
    SAW::Logging
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
)
set_target_properties(saw_terminal PROPERTIES OUTPUT_NAME "SmartAccessWelcome")

install(TARGETS saw_terminal RUNTIME DESTINATION bin)
install(FILES ${PROJECT_SOURCE_DIR}/config/config.example.json DESTINATION config)
