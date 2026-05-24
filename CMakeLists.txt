cmake_minimum_required(VERSION 3.16)

project(ShiShiGa-Workspace)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets)

qt_standard_project_setup()

set_source_files_properties(resources/qt.conf PROPERTIES QT_RESOURCE_ALIAS "qt.conf")

add_executable(ShiShiGa-Workspace WIN32
    main.cpp
    src/platform/WinEnginePaths.cpp
    src/platform/WinEnginePaths.h
    src/profile/ProfileManager.cpp
    src/profile/ProfileManager.h
    src/profile/ProfileSecurity.cpp
    src/profile/ProfileSecurity.h
    src/services/AiService.h
)

qt_add_resources(ShiShiGa-Workspace "embedded_qt_conf"
    PREFIX "/qt/etc"
    FILES
        resources/qt.conf
)

target_include_directories(ShiShiGa-Workspace PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ShiShiGa-Workspace PRIVATE
    Qt6::WebEngineWidgets
)

if(WIN32)
    target_link_libraries(ShiShiGa-Workspace PRIVATE advapi32)
endif()

qt_finalize_executable(ShiShiGa-Workspace)
