add_executable(saw_camera_demo WIN32
    main.cpp
    simulation_window.cpp
    simulation_window.h
)

target_link_libraries(saw_camera_demo PRIVATE
    SAW::Camera
    SAW::Device
    SAW::AccessServer
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
)

set_target_properties(saw_camera_demo PROPERTIES OUTPUT_NAME "CameraSimulationDemo")
install(TARGETS saw_camera_demo RUNTIME DESTINATION bin)
