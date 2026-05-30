cmake_minimum_required(VERSION 3.16)

project(ShiShiGa-Workspace VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets Concurrent)

qt_standard_project_setup()

set_source_files_properties(resources/qt.conf PROPERTIES QT_RESOURCE_ALIAS "qt.conf")

add_executable(ShiShiGa-Workspace WIN32
    main.cpp
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
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
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

target_include_directories(ShiShiGa-Workspace PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(ShiShiGa-Workspace PRIVATE
    SHISHIGA_APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(ShiShiGa-Workspace PRIVATE
    Qt6::Concurrent
    Qt6::WebEngineWidgets
)

if(WIN32)
    target_link_libraries(ShiShiGa-Workspace PRIVATE advapi32 dbghelp psapi)
endif()

qt_finalize_executable(ShiShiGa-Workspace)
