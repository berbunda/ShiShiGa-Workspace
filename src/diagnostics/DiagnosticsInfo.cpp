#include "DiagnosticsInfo.h"

#include "logging/CrashLogger.h"
#include "profile/ProfileManager.h"
#include "services/ServiceRegistry.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSysInfo>
#include <QWebEngineView>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace {

QString na()
{
    return QStringLiteral("N/A");
}

QString joinPaths(const QStringList &paths)
{
    if (paths.isEmpty())
        return na();
    return paths.join(QStringLiteral("\n"));
}

QString buildTypeLabel()
{
#ifdef QT_DEBUG
    return QStringLiteral("Debug");
#else
    return QStringLiteral("Release");
#endif
}

QString processRamUsageLabel()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS counters = {};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) == FALSE)
        return na();

    const double megabytes = static_cast<double>(counters.WorkingSetSize) / (1024.0 * 1024.0);
    return QStringLiteral("%1 MB").arg(QString::number(megabytes, 'f', 1));
#else
    return na();
#endif
}

} // namespace

DebugDiagnosticsSnapshot DiagnosticsInfo::collect(const ServiceManager *serviceManager)
{
    DebugDiagnosticsSnapshot snapshot;

    snapshot.application.applicationName = QCoreApplication::applicationName().isEmpty()
        ? QStringLiteral("ShiShiga Workspace")
        : QCoreApplication::applicationName();
    snapshot.application.applicationVersion = CrashLogger::instance().installedVersion().isEmpty()
        ? na()
        : CrashLogger::instance().installedVersion();
    snapshot.application.qtVersion = QStringLiteral(QT_VERSION_STR);
    snapshot.application.buildType = buildTypeLabel();
    snapshot.application.platform = QSysInfo::prettyProductName().isEmpty()
        ? QSysInfo::productType()
        : QSysInfo::prettyProductName();
    snapshot.application.architecture = QSysInfo::currentCpuArchitecture().isEmpty()
        ? QSysInfo::buildCpuArchitecture()
        : QSysInfo::currentCpuArchitecture();
    snapshot.application.executablePath = QCoreApplication::applicationFilePath().isEmpty()
        ? na()
        : QDir::toNativeSeparators(QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath());

    snapshot.runtime.libraryPaths = joinPaths(QCoreApplication::libraryPaths());
    snapshot.runtime.pluginPaths = joinPaths(QCoreApplication::libraryPaths());
    snapshot.runtime.applicationDirectory = CrashLogger::instance().applicationDirectoryPath().isEmpty()
        ? na()
        : QDir::toNativeSeparators(CrashLogger::instance().applicationDirectoryPath());
    snapshot.runtime.profilesRootDirectory = QDir::toNativeSeparators(ProfileManager::instance().profilesRoot());
    snapshot.runtime.crashLogsDirectory = CrashLogger::instance().crashesDirectoryPath().isEmpty()
        ? na()
        : QDir::toNativeSeparators(CrashLogger::instance().crashesDirectoryPath());

    if (serviceManager != nullptr && !serviceManager->activeServiceId().isEmpty()) {
        const QString serviceId = serviceManager->activeServiceId();
        const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(serviceId);
        const QString profileId = catalog.profileId.isEmpty() ? catalog.id : catalog.profileId;

        snapshot.activeService.hasActiveService = true;
        snapshot.activeService.serviceName = catalog.displayName.isEmpty() ? serviceId : catalog.displayName;
        snapshot.activeService.serviceState = serviceManager->profileRuntimeStateLabel(serviceId);
        snapshot.activeService.profileName = profileId;
        snapshot.activeService.profilePath = QDir::toNativeSeparators(
            ProfileManager::instance().profileRootPath(profileId));

        if (QWebEngineView *view = serviceManager->viewFor(serviceId); view != nullptr) {
            snapshot.activeService.serviceUrl = view->url().toString();
        } else if (catalog.defaultUrl.isValid()) {
            snapshot.activeService.serviceUrl = catalog.defaultUrl.toString();
        } else {
            snapshot.activeService.serviceUrl = na();
        }
    }

    snapshot.memory.processRamUsage = processRamUsageLabel();
    snapshot.memory.loadedServices = serviceManager != nullptr ? serviceManager->loadedServiceCount() : 0;
    snapshot.memory.unloadedServices = serviceManager != nullptr ? serviceManager->unloadedServiceCount() : 0;

    return snapshot;
}

QString DiagnosticsInfo::formatDebugReport(const DebugDiagnosticsSnapshot &snapshot)
{
    QString report;

    auto appendSection = [&report](const QString &title) {
        report += title;
        report += QStringLiteral("\n");
        report += QString(40, QLatin1Char('-'));
        report += QStringLiteral("\n");
    };

    auto appendLine = [&report](const QString &key, const QString &value) {
        report += key;
        report += QStringLiteral(": ");
        report += value;
        report += QStringLiteral("\n");
    };

    appendSection(QStringLiteral("Application"));
    appendLine(QStringLiteral("Application name"), snapshot.application.applicationName);
    appendLine(QStringLiteral("Application version"), snapshot.application.applicationVersion);
    appendLine(QStringLiteral("Qt version"), snapshot.application.qtVersion);
    appendLine(QStringLiteral("Build type"), snapshot.application.buildType);
    appendLine(QStringLiteral("Platform"), snapshot.application.platform);
    appendLine(QStringLiteral("Architecture"), snapshot.application.architecture);
    appendLine(QStringLiteral("Executable path"), snapshot.application.executablePath);
    report += QStringLiteral("\n");

    appendSection(QStringLiteral("Runtime"));
    appendLine(QStringLiteral("Qt library paths"), snapshot.runtime.libraryPaths);
    appendLine(QStringLiteral("Plugin paths"), snapshot.runtime.pluginPaths);
    appendLine(QStringLiteral("Application directory"), snapshot.runtime.applicationDirectory);
    appendLine(QStringLiteral("Profiles root directory"), snapshot.runtime.profilesRootDirectory);
    appendLine(QStringLiteral("Crash logs directory"), snapshot.runtime.crashLogsDirectory);
    report += QStringLiteral("\n");

    appendSection(QStringLiteral("Active Service"));
    if (snapshot.activeService.hasActiveService) {
        appendLine(QStringLiteral("Service name"), snapshot.activeService.serviceName);
        appendLine(QStringLiteral("Service URL"), snapshot.activeService.serviceUrl);
        appendLine(QStringLiteral("Service state"), snapshot.activeService.serviceState);
        appendLine(QStringLiteral("Profile name"), snapshot.activeService.profileName);
        appendLine(QStringLiteral("Profile path"), snapshot.activeService.profilePath);
    } else {
        appendLine(QStringLiteral("Status"), QStringLiteral("No active service"));
    }
    report += QStringLiteral("\n");

    appendSection(QStringLiteral("Memory"));
    appendLine(QStringLiteral("Process RAM usage"), snapshot.memory.processRamUsage);
    appendLine(QStringLiteral("Loaded services"), QString::number(snapshot.memory.loadedServices));
    appendLine(QStringLiteral("Unloaded services"), QString::number(snapshot.memory.unloadedServices));

    return report;
}

QString DiagnosticsInfo::profileFolderToOpen(const ServiceManager *serviceManager)
{
    if (serviceManager != nullptr && !serviceManager->activeServiceId().isEmpty()) {
        const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(serviceManager->activeServiceId());
        const QString profileId = catalog.profileId.isEmpty() ? catalog.id : catalog.profileId;
        const QString profilePath = ProfileManager::instance().profileRootPath(profileId);
        if (!profilePath.isEmpty())
            return profilePath;
    }

    return ProfileManager::instance().profilesRoot();
}
