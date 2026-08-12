add_library(saw_access_server access_server.cpp)
add_library(SAW::AccessServer ALIAS saw_access_server)
target_include_directories(saw_access_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_access_server PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
)
