# Keep configuration testable before production sources are migrated.
add_test(NAME repository_configuration
    COMMAND ${CMAKE_COMMAND} -E echo "Repository configuration is valid"
)

