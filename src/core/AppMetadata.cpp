#include "AppMetadata.h"

#include <QtGlobal>
#include <QSysInfo>

QString AppMetadata::applicationVersion()
{
#ifdef SHISHIGA_APP_VERSION
    return QStringLiteral(SHISHIGA_APP_VERSION);
#else
    return QStringLiteral("unknown");
#endif
}

QString AppMetadata::gitCommit()
{
#ifdef SHISHIGA_GIT_COMMIT
    return QStringLiteral(SHISHIGA_GIT_COMMIT);
#else
    return QStringLiteral("unknown");
#endif
}

QString AppMetadata::buildDate()
{
#ifdef SHISHIGA_BUILD_DATE
    return QStringLiteral(SHISHIGA_BUILD_DATE);
#else
    return QStringLiteral("unknown");
#endif
}

QString AppMetadata::platformDescription()
{
    QString osName;
#if defined(Q_OS_WIN)
    osName = QStringLiteral("Windows");
#elif defined(Q_OS_LINUX)
    osName = QStringLiteral("Linux");
#elif defined(Q_OS_MACOS)
    osName = QStringLiteral("macOS");
#else
    osName = QSysInfo::productType();
#endif

    QString archLabel;
#if defined(Q_PROCESSOR_ARM_64)
    archLabel = QStringLiteral("arm64");
#elif defined(Q_PROCESSOR_X86_64)
    archLabel = QStringLiteral("x64");
#elif defined(Q_PROCESSOR_X86_32)
    archLabel = QStringLiteral("x86");
#else
    archLabel = QSysInfo::currentCpuArchitecture();
#endif

    return QStringLiteral("%1 %2").arg(osName, archLabel);
}

QString AppMetadata::qtRuntimeVersion()
{
    return QString::fromLatin1(qVersion());
}

QString AppMetadata::compilerLabel()
{
#ifdef SHISHIGA_COMPILER_LABEL
    return QStringLiteral(SHISHIGA_COMPILER_LABEL);
#else
    return QStringLiteral("unknown");
#endif
}

QString AppMetadata::cmakeVersionLabel()
{
#ifdef SHISHIGA_CMAKE_VERSION_LABEL
    return QStringLiteral(SHISHIGA_CMAKE_VERSION_LABEL);
#else
    return QStringLiteral("unknown");
#endif
}

QString AppMetadata::versionInfoClipboardText()
{
    return QStringLiteral(
               "%1\n"
               "Version: %2\n"
               "Commit: %3\n"
               "Build date: %4\n"
               "Platform: %5\n"
               "Qt: %6\n"
               "GitHub: %7")
        .arg(QString::fromLatin1(kApplicationDisplayName),
             applicationVersion(),
             gitCommit(),
             buildDate(),
             platformDescription(),
             qtRuntimeVersion(),
             QString::fromLatin1(kGitHubRepositoryUrl));
}

