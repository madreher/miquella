add_executable(MiquellaClient client.cpp)

target_link_libraries(MiquellaClient
                                MQ_project_libraries
                                MQ_project_options
                                MQ_project_warnings
                                ${PROJECT_NAME}::imgui
                                MiquellaLib
                     )
install(TARGETS
            MiquellaClient
        DESTINATION
            ${MQ_BIN_DIR}
        )
