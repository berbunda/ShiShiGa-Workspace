#pragma once

#include "core/ServiceManager.h"

#include <QString>

struct ApplicationDiagnostics
{
    QString applicationName;
    QString applicationVersion;
    QString qtVersion;
    QString buildType;
    QString platform;
    QString architecture;
    QString executablePath;
};

struct RuntimeDiagnostics
{
    QString libraryPaths;
    QString pluginPaths;
    QString applicationDirectory;
    QString profilesRootDirectory;
    QString crashLogsDirectory;
};

struct ActiveServiceDiagnostics
{
    bool hasActiveService = false;
    QString serviceName;
    QString serviceUrl;
    QString serviceState;
    QString profileName;
    QString profilePath;
};

struct MemoryDiagnostics
{
    QString processRamUsage;
    int loadedServices = 0;
    int unloadedServices = 0;
};

struct DebugDiagnosticsSnapshot
{
    ApplicationDiagnostics application;
    RuntimeDiagnostics runtime;
    ActiveServiceDiagnostics activeService;
    MemoryDiagnostics memory;
};

class DiagnosticsInfo
{
public:
    static DebugDiagnosticsSnapshot collect(const ServiceManager *serviceManager);
    static QString formatDebugReport(const DebugDiagnosticsSnapshot &snapshot);
    static QString profileFolderToOpen(const ServiceManager *serviceManager);
};
