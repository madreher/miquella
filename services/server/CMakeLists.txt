add_executable(MiquellaServer server.cpp)

target_link_libraries(MiquellaServer
                                MQ_project_libraries
                                MQ_project_options
                                MQ_project_warnings
                                ${PROJECT_NAME}::imgui
                     )
install(TARGETS
            MiquellaServer
        DESTINATION
            ${MQ_BIN_DIR}
        )
