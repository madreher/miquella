add_executable(MainBenchmark mainBenchmark.cpp)

target_link_libraries(MainBenchmark
                                MQ_project_libraries
                                MQ_project_options
                                MQ_project_warnings
                                ${PROJECT_NAME}::imgui
                                MiquellaLib
                                CONAN_PKG::benchmark
                     )
install(TARGETS
            MainBenchmark
        DESTINATION
            ${MQ_BIN_DIR}
        )