add_library(saw_config app_config.cpp)
add_library(SAW::Config ALIAS saw_config)
target_include_directories(saw_config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_config PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)
