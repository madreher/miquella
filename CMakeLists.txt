cmake_minimum_required(VERSION 3.15)


################################################################################
include(version.cmake)
################################################################################

# Set the project name to your project name, my project isn't very descriptive
project(Miquella
        VERSION
            ${MQ_VERSION_MAJOR}.${MQ_VERSION_MINOR}.${MQ_VERSION_PATCH}
        )

if( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake )
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(NO_OUTPUT_DIRS TARGETS)
else()
    message(FATAL_ERROR "\n\nERROR: \nConan config file not found. You need to call the following from the build directory first: \n  conan profile new qt_cmake_autodetect --detect --force\n  conan install -s compiler.libcxx=libstdc++11  ${CMAKE_SOURCE_DIR} --build missing")
endif()

#########################################################
# Other targets
#########################################################
add_custom_target( ${PROJECT_NAME}_other_files
                        SOURCES
                           README.md
                           run_cmake_script.sh
                           conanfile.txt
                           cmake/CompilerWarnings.cmake
                           cmake/Coverage.cmake
                           cmake/StandardProjectSettings.cmake
                           .gitignore
                )
#########################################################

include(GNUInstallDirs)
include(cmake/StandardProjectSettings.cmake)

#######################################################################################
# standard compiler warnings
add_library(MQ_project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(MQ_project_warnings)
#######################################################################################

#######################################################################################
# Link this 'library' to set the c++ standard / compile-time options requested
add_library(MQ_project_options INTERFACE)
#######################################################################################
target_compile_features(MQ_project_options INTERFACE cxx_std_20)

target_compile_definitions(MQ_project_options
                            INTERFACE
                                CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
                                CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
                                MQ_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                                MQ_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                MQ_VERSION_STRING="${MQ_VERSION_STRING}"
                                MQ_VERSION_MAJOR=${MQ_VERSION_MAJOR}
                                MQ_VERSION_MINOR=${MQ_VERSION_MINOR}
                                MQ_VERSION_PATCH=${MQ_VERSION_PATCH}
                                MQ_PACKAGE_SUFFIX="${NFCBN_PKG_SUFFIX}"
                                MQ_VERSION_TWEAK=${MQ_VERSION_BUILD}
                                CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                                MQ_GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

#######################################################################################
# Link this interface to include a standard set of libs that would be needed
# for developing Nanodesigner
#######################################################################################
add_library(MQ_project_libraries INTERFACE )
#######################################################################################

target_include_directories( MQ_project_libraries
    INTERFACE
        ${CMAKE_SOURCE_DIR}/include)
if(WIN32)
target_link_libraries( MQ_project_libraries
    INTERFACE
        CONAN_PKG::glm
        CONAN_PKG::spdlog
	    CONAN_PKG::sdl
        CONAN_PKG::lyra
        CONAN_PKG::glbinding
        CONAN_PKG::bshoshany-thread-pool
        CONAN_PKG::cpr
)
else()
target_link_libraries( MQ_project_libraries
    INTERFACE
        CONAN_PKG::glm
        CONAN_PKG::spdlog
	    CONAN_PKG::sdl
        CONAN_PKG::lyra
        CONAN_PKG::glbinding
        CONAN_PKG::bshoshany-thread-pool
        CONAN_PKG::cpr
        CONAN_PKG::nlohmann_json
        stdc++fs
)
endif()

#########################################################
# Build IMGUI
#########################################################
set(lib_name   ${PROJECT_NAME}_imgui)
set(lib_alias  ${PROJECT_NAME}::imgui)
# IMGUI source recommends to always build static
add_library( ${lib_name}
                    thirdParty/imgui/imgui.cpp
                    thirdParty/imgui/imgui_draw.cpp
                    thirdParty/imgui/imgui_demo.cpp
                    thirdParty/imgui/imgui_tables.cpp
                    thirdParty/imgui/imgui_widgets.cpp
                    thirdParty/imgui/misc/cpp/imgui_stdlib.cpp
                    thirdParty/imgui/backends/imgui_impl_sdl2.cpp
                    thirdParty/imgui/backends/imgui_impl_opengl3.cpp
                    )
target_compile_features(${lib_name} INTERFACE cxx_std_17)
add_library( ${lib_alias} ALIAS ${lib_name} )
target_include_directories( ${lib_name}
                               SYSTEM
                               INTERFACE
                                   "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>"

                  )
target_link_libraries( ${lib_name}
                        PRIVATE
                        CONAN_PKG::glm
                        CONAN_PKG::sdl)

target_include_directories( ${lib_name}
                             SYSTEM
                             INTERFACE
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/imgui>"
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/imgui/backends>"
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/ImGuiFileDialog>"
                )
target_include_directories( ${lib_name}
                             SYSTEM
                             PRIVATE
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/imgui>"
                             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/imgui/backends>"

                )

set(MQ_BIN_DIR 			${CMAKE_INSTALL_PREFIX}/bin)
set(MQ_LIB_DIR 			${CMAKE_INSTALL_PREFIX}/lib)

add_subdirectory(src)
add_subdirectory(services)
