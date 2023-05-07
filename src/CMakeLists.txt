add_executable(RayTracer main.cpp)

target_link_libraries(RayTracer
                              	rt_project_libraries
				rt_project_options
                     )
install(TARGETS
            RayTracer
        DESTINATION
            ${RT_BIN_DIR}
        )
