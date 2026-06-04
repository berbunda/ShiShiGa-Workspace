cmake_minimum_required(VERSION 3.16)

project(ShiShiGa-Workspace VERSION 0.6.2)

include(cmake/AppVersion.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets Concurrent)

qt_standard_project_setup()

set_source_files_properties(resources/qt.conf PROPERTIES QT_RESOURCE_ALIAS "qt.conf")

add_executable(ShiShiGa-Workspace WIN32
    main.cpp
    src/core/AppIcons.cpp
    src/core/AppIcons.h
    src/core/AppMetadata.cpp
    src/core/AppMetadata.h
    src/core/LicenseText.cpp
    src/core/LicenseText.h
    src/core/ServiceFaviconProvider.cpp
    src/core/ServiceFaviconProvider.h
    src/core/ServiceManager.cpp
    src/core/ServiceManager.h
    src/core/SettingsManager.cpp
    src/core/SettingsManager.h
    src/core/UserAgentSettings.cpp
    src/core/UserAgentSettings.h
    src/diagnostics/DiagnosticsInfo.cpp
    src/diagnostics/DiagnosticsInfo.h
    src/diagnostics/DiagnosticsWebEnginePage.cpp
    src/diagnostics/DiagnosticsWebEnginePage.h
    src/diagnostics/WebEngineConsoleLog.cpp
    src/diagnostics/WebEngineConsoleLog.h
    src/diagnostics/WebEngineDiagnosticsInfo.cpp
    src/diagnostics/WebEngineDiagnosticsInfo.h
    src/logging/CrashLogger.cpp
    src/logging/CrashLogger.h
    src/platform/WinEnginePaths.cpp
    src/platform/WinEnginePaths.h
    src/profile/ProfileManager.cpp
    src/profile/ProfileManager.h
    src/profile/ProfileSecurity.cpp
    src/profile/ProfileSecurity.h
    src/services/ServiceRegistry.cpp
    src/services/ServiceRegistry.h
    src/services/ServiceState.h
    src/ui/AboutWindow.cpp
    src/ui/AboutWindow.h
    src/ui/ApplicationTray.cpp
    src/ui/ApplicationTray.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/LicenseWindow.cpp
    src/ui/LicenseWindow.h
    src/ui/DebugConsoleWindow.cpp
    src/ui/DebugConsoleWindow.h
    src/ui/DevConsoleWindow.cpp
    src/ui/DevConsoleWindow.h
    src/ui/SettingsWindow.cpp
    src/ui/SettingsWindow.h
    src/ui/UserAgentSettingsWidget.cpp
    src/ui/UserAgentSettingsWidget.h
    src/ui/ProfileManagerWidget.cpp
    src/ui/ProfileManagerWidget.h
    src/ui/ServiceButton.cpp
    src/ui/ServiceButton.h
    src/ui/ServiceSidebar.cpp
    src/ui/ServiceSidebar.h
    src/ui/SidebarAddButton.cpp
    src/ui/SidebarAddButton.h
)

qt_add_resources(ShiShiGa-Workspace "embedded_qt_conf"
    PREFIX "/qt/etc"
    FILES
        resources/qt.conf
)

qt_add_resources(ShiShiGa-Workspace "embedded_license"
    PREFIX "/"
    FILES
        LICENSE
)

set(SHISHIGA_ICON_DIR "${CMAKE_SOURCE_DIR}/resources/icons")
set_source_files_properties("${SHISHIGA_ICON_DIR}/ShiShiGa-128.png"
    PROPERTIES QT_RESOURCE_ALIAS "ShiShiGa-128.png")
set_source_files_properties("${SHISHIGA_ICON_DIR}/ShiShiGa-tray-48.png"
    PROPERTIES QT_RESOURCE_ALIAS "ShiShiGa-tray-48.png")
set_source_files_properties("${SHISHIGA_ICON_DIR}/ShiShiGa-tray-16.png"
    PROPERTIES QT_RESOURCE_ALIAS "ShiShiGa-tray-16.png")

qt_add_resources(ShiShiGa-Workspace "embedded_app_icons"
    PREFIX "/icons"
    FILES
        "${SHISHIGA_ICON_DIR}/ShiShiGa-128.png"
        "${SHISHIGA_ICON_DIR}/ShiShiGa-tray-48.png"
        "${SHISHIGA_ICON_DIR}/ShiShiGa-tray-16.png"
)

if(WIN32)
    set(SHISHIGA_EXE_ICON "${CMAKE_SOURCE_DIR}/resources/icons/ShiShiGa.ico")
    target_sources(ShiShiGa-Workspace PRIVATE resources/windows/app.rc)
    set_source_files_properties(resources/windows/app.rc PROPERTIES
        OBJECT_DEPENDS "${SHISHIGA_EXE_ICON}")
endif()

add_custom_command(TARGET ShiShiGa-Workspace POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/LICENSE"
        "$<TARGET_FILE_DIR:ShiShiGa-Workspace>/LICENSE"
    COMMENT "Copy LICENSE next to executable for development runs"
)

target_include_directories(ShiShiGa-Workspace PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(ShiShiGa-Workspace PRIVATE
    "SHISHIGA_APP_VERSION=\"${PROJECT_VERSION}\""
    "SHISHIGA_GIT_COMMIT=\"${SHISHIGA_GIT_COMMIT}\""
    "SHISHIGA_BUILD_DATE=\"${SHISHIGA_BUILD_DATE}\""
    "SHISHIGA_COMPILER_LABEL=\"${SHISHIGA_COMPILER_LABEL}\""
    "SHISHIGA_CMAKE_VERSION_LABEL=\"${SHISHIGA_CMAKE_VERSION_LABEL}\""
)

target_link_libraries(ShiShiGa-Workspace PRIVATE
    Qt6::Concurrent
    Qt6::WebEngineWidgets
)

if(WIN32)
    target_link_libraries(ShiShiGa-Workspace PRIVATE advapi32 dbghelp psapi)
endif()

qt_finalize_executable(ShiShiGa-Workspace)
