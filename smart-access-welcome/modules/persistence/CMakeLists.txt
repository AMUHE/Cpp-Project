add_library(saw_persistence access_event_store.cpp)
add_library(SAW::Persistence ALIAS saw_persistence)
target_include_directories(saw_persistence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_persistence PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Sql)
