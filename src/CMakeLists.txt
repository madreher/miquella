add_executable(MiquellaSDL main.cpp)

target_link_libraries(MiquellaSDL
                                MQ_project_libraries
                                MQ_project_options
                                MQ_project_warnings
                                ${PROJECT_NAME}::imgui
                     )
install(TARGETS
            MiquellaSDL
        DESTINATION
            ${MQ_BIN_DIR}
        )
