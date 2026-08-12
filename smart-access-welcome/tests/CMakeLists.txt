# Keep configuration testable before production sources are migrated.
add_test(NAME repository_configuration
    COMMAND ${CMAKE_COMMAND} -E echo "Repository configuration is valid"
)

if(TARGET SAW::AccessServer)
    add_executable(access_server_test access_server_test.cpp)
    target_link_libraries(access_server_test PRIVATE
        SAW::AccessServer
        Qt${QT_VERSION_MAJOR}::Test
    )
    add_test(NAME access_server COMMAND access_server_test)
endif()

if(TARGET SAW::AccessControl)
    add_executable(access_policy_test access_policy_test.cpp)
    target_link_libraries(access_policy_test PRIVATE SAW::AccessControl)
    add_test(NAME access_policy COMMAND access_policy_test)
endif()

if(TARGET SAW::Config AND TARGET SAW::Persistence AND TARGET SAW::Device)
    add_executable(infrastructure_test infrastructure_test.cpp)
    target_link_libraries(infrastructure_test PRIVATE
        SAW::Config SAW::Persistence SAW::Device Qt${QT_VERSION_MAJOR}::Test)
    add_test(NAME infrastructure COMMAND infrastructure_test)
endif()

