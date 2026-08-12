add_library(saw_logging json_logger.cpp)
add_library(SAW::Logging ALIAS saw_logging)
target_include_directories(saw_logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_logging PUBLIC Qt${QT_VERSION_MAJOR}::Core)
