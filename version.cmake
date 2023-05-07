# Do not change this value
set(RT_VERSION_MAJOR  2     CACHE NUMBER "Major Version Number")

# Change this if you have added a new feature
set(RT_VERSION_MINOR  3     CACHE NUMBER "Minor Version Number")

# Change this if you have fixed a bug
set(RT_VERSION_PATCH  3     CACHE NUMBER "Patch NUMBER")

execute_process(COMMAND            git rev-list --count HEAD
                WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE    git_commit_count
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

# This is only changed through the CI system
set(RT_VERSION_BUILD  ${git_commit_count}     CACHE NUMBER "Build version number. This value should be equal to the number of git commits using: git rev-list --count HEAD")
set(RT_VERSION_STRING ${RT_VERSION_MAJOR}.${RT_VERSION_MINOR}.${RT_VERSION_PATCH}.${RT_VERSION_BUILD} CACHE STRING "The final version number")
