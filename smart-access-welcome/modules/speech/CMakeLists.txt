add_library(saw_speech speech_announcer.cpp)
add_library(SAW::Speech ALIAS saw_speech)
target_include_directories(saw_speech PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(saw_speech PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::TextToSpeech)
