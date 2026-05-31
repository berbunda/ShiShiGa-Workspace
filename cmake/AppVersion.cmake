# Populates SHISHIGA_GIT_COMMIT and SHISHIGA_BUILD_DATE for compile-time metadata.

set(SHISHIGA_GIT_COMMIT "unknown")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _shishiga_git_commit_raw
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _shishiga_git_result
    )
    if(_shishiga_git_result EQUAL 0 AND _shishiga_git_commit_raw)
        set(SHISHIGA_GIT_COMMIT "${_shishiga_git_commit_raw}")
    endif()
endif()

string(TIMESTAMP SHISHIGA_BUILD_DATE UTC "%Y-%m-%dT%H:%M:%SZ")

if(MSVC)
    set(SHISHIGA_COMPILER_LABEL "MSVC 2022")
else()
    set(SHISHIGA_COMPILER_LABEL "${CMAKE_CXX_COMPILER_ID}")
endif()

set(SHISHIGA_CMAKE_VERSION_LABEL "CMake ${CMAKE_VERSION}")
