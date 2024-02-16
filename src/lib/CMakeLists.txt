#=======================================
set(library_MODULE MiquellaLib   )
#=======================================

file(GLOB_RECURSE files "*.cpp")

add_library( ${library_MODULE} SHARED ${files} )


target_include_directories( ${library_MODULE}
                            PUBLIC
                               "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
                           )

target_link_libraries( ${library_MODULE}
                            PUBLIC
                                MQ_project_libraries
                                MQ_project_options
                                MQ_project_warnings
                                ${PROJECT_NAME}::imgui
                     )

target_link_libraries( ${library_MODULE}
                         PRIVATE
                            MQ_project_warnings
                            MQ_project_options
                  )
target_compile_features(   ${library_MODULE} PUBLIC cxx_std_20)