cmake_minimum_required(VERSION 3.15)


################################################################################
include(version.cmake)
################################################################################

# Set the project name to your project name, my project isn't very descriptive
project(raytracer
        VERSION
            ${RT_VERSION_MAJOR}.${RT_VERSION_MINOR}.${RT_VERSION_PATCH}
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
# Link this 'library' to set the c++ standard / compile-time options requested
add_library(rt_project_options INTERFACE)
#######################################################################################
target_compile_features(rt_project_options INTERFACE cxx_std_17)

target_compile_definitions(rt_project_options
                            INTERFACE
                                CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
                                CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
                                RT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                                RT_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                RT_VERSION_STRING="${RT_VERSION_STRING}"
                                RT_VERSION_MAJOR=${RT_VERSION_MAJOR}
                                RT_VERSION_MINOR=${RT_VERSION_MINOR}
                                RT_VERSION_PATCH=${RT_VERSION_PATCH}
                                RT_PACKAGE_SUFFIX="${NFCBN_PKG_SUFFIX}"
                                RT_VERSION_TWEAK=${RT_VERSION_BUILD}
                                CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                                RT_GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

#######################################################################################
# Link this interface to include a standard set of libs that would be needed
# for developing Nanodesigner
#######################################################################################
add_library(rt_project_libraries INTERFACE )
#######################################################################################

target_link_libraries( rt_project_libraries
    INTERFACE
        CONAN_PKG::glm
        CONAN_PKG::spdlog
        stdc++fs
)

set(RT_BIN_DIR 			${CMAKE_INSTALL_PREFIX}/bin)
set(RT_LIB_DIR 			${CMAKE_INSTALL_PREFIX}/lib)

add_subdirectory(src)
