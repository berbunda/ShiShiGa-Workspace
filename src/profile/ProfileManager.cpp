#include "ProfileManager.h"

#include "ProfileSecurity.h"
#include "core/UserAgentSettings.h"
#include "services/ServiceRegistry.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

Q_LOGGING_CATEGORY(profileLog, "shishiga.profile")

ProfileManager &ProfileManager::instance()
{
    static ProfileManager manager;
    return manager;
}

ProfileManager::ProfileManager(QObject *parent)
    : QObject(parent)
{
}

QList<ManagedProfileInfo> ProfileManager::registeredProfiles() const
{
    QList<ManagedProfileInfo> profiles;
    profiles.reserve(ServiceRegistry::catalog().size());

    for (const ServiceCatalogEntry &entry : ServiceRegistry::catalog()) {
        const QString profileId = entry.profileId.isEmpty() ? entry.id : entry.profileId;
        const QString normalized = normalizeServiceName(profileId);
        if (normalized.isEmpty())
            continue;

        ManagedProfileInfo info;
        info.serviceId = entry.id;
        info.serviceDisplayName = entry.displayName;
        info.profileId = normalized;
        info.profileRootPath = profileRootPath(normalized);
        info.storagePath = QDir(info.profileRootPath).filePath(QStringLiteral("storage"));
        info.cachePath = QDir(info.profileRootPath).filePath(QStringLiteral("cache"));
        profiles.append(info);
    }

    return profiles;
}

QString ProfileManager::profilesRoot() const
{
    const QString localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    return QDir(localAppData).filePath(QStringLiteral("ShiShiga-Workspace/profiles"));
}

QString ProfileManager::profileRootPath(const QString &profileId) const
{
    const QString normalized = normalizeServiceName(profileId);
    if (normalized.isEmpty())
        return {};

    return QDir(profilesRoot()).filePath(normalized);
}

ProfileDataState ProfileManager::dataState(const QString &profileId) const
{
    const QString root = profileRootPath(profileId);
    if (root.isEmpty() || !QDir(root).exists())
        return ProfileDataState::Missing;

    const QString storagePath = QDir(root).filePath(QStringLiteral("storage"));
    const QString cachePath = QDir(root).filePath(QStringLiteral("cache"));

    if (directoryContainsUserData(storagePath) || directoryContainsUserData(cachePath))
        return ProfileDataState::Present;

    if (QDir(storagePath).exists() || QDir(cachePath).exists())
        return ProfileDataState::Empty;

    return ProfileDataState::Missing;
}

bool ProfileManager::hasActiveProfile(const QString &profileId) const
{
    const QString normalized = normalizeServiceName(profileId);
    return !normalized.isEmpty() && m_profiles.contains(normalized);
}

qint64 ProfileManager::directorySizeOnDisk(const QString &directoryPath) const
{
    qint64 totalBytes = 0;
    QDirIterator iterator(directoryPath,
                          QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        const QFileInfo info = iterator.fileInfo();
        if (info.isFile())
            totalBytes += info.size();
    }
    return totalBytes;
}

void ProfileManager::releaseProfile(const QString &profileId)
{
    const QString normalized = normalizeServiceName(profileId);
    if (normalized.isEmpty())
        return;

    QWebEngineProfile *profile = m_profiles.take(normalized);
    if (profile == nullptr)
        return;

    qCInfo(profileLog) << "Releasing in-memory profile" << normalized;
    delete profile;
}

bool ProfileManager::clearProfileData(const QString &profileId, QString *errorMessage)
{
    const QString normalized = normalizeServiceName(profileId);
    if (normalized.isEmpty()) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Invalid profile identifier.");
        return false;
    }

    if (hasActiveProfile(normalized)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Profile is still loaded in memory. Unload the service first.");
        }
        return false;
    }

    const QString root = profileRootPath(normalized);
    const QString storagePath = QDir(root).filePath(QStringLiteral("storage"));
    const QString cachePath = QDir(root).filePath(QStringLiteral("cache"));

    if (!removeDirectoryTree(storagePath, errorMessage))
        return false;
    if (!removeDirectoryTree(cachePath, errorMessage))
        return false;

    QString recreatedStorage;
    QString recreatedCache;
    if (!ensureProfileDirectories(normalized, &recreatedStorage, &recreatedCache)) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Failed to recreate profile directories.");
        return false;
    }

    qCInfo(profileLog) << "Cleared Chromium profile data for" << normalized;
    emit profileDataCleared(normalized);
    return true;
}

QString ProfileManager::normalizeServiceName(const QString &serviceName) const
{
    const QString normalized = serviceName.trimmed().toLower();
    static const QRegularExpression validPattern(QStringLiteral("^[a-z0-9]+$"));

    if (!validPattern.match(normalized).hasMatch()) {
        qCWarning(profileLog) << "Invalid service name:" << serviceName
                              << "- expected lowercase alphanumeric identifier";
        return {};
    }

    return normalized;
}

bool ProfileManager::ensureProfileDirectories(const QString &serviceName,
                                              QString *storagePath,
                                              QString *cachePath) const
{
    const QString profileRoot = QDir(profilesRoot()).filePath(serviceName);
    const QString storage = QDir(profileRoot).filePath(QStringLiteral("storage"));
    const QString cache = QDir(profileRoot).filePath(QStringLiteral("cache"));

    QDir dir;
    if (!dir.mkpath(storage) || !dir.mkpath(cache)) {
        qCWarning(profileLog) << "Failed to create profile directories for" << serviceName;
        return false;
    }

    if (!ProfileSecurity::restrictToCurrentUserAndAdministrators(profileRoot)
        || !ProfileSecurity::restrictToCurrentUserAndAdministrators(storage)
        || !ProfileSecurity::restrictToCurrentUserAndAdministrators(cache)) {
        qCWarning(profileLog) << "Failed to apply directory ACL for" << serviceName;
        return false;
    }

    *storagePath = storage;
    *cachePath = cache;
    return true;
}

QWebEngineProfile *ProfileManager::createProfile(const QString &serviceName)
{
    QString storagePath;
    QString cachePath;
    if (!ensureProfileDirectories(serviceName, &storagePath, &cachePath))
        return nullptr;

    auto *profile = new QWebEngineProfile(serviceName, this);
    profile->setPersistentStoragePath(storagePath);
    profile->setCachePath(cachePath);
    profile->setHttpCacheType(QWebEngineProfile::HttpCacheType::DiskHttpCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    UserAgentSettings::applyToProfile(profile);

    qCInfo(profileLog) << "Created persistent profile for" << serviceName
                       << "storage:" << storagePath
                       << "cache:" << cachePath;

    return profile;
}

QWebEngineProfile *ProfileManager::profileFor(const QString &serviceName)
{
    const QString normalized = normalizeServiceName(serviceName);
    if (normalized.isEmpty())
        return nullptr;

    if (m_profiles.contains(normalized))
        return m_profiles.value(normalized);

    QWebEngineProfile *profile = createProfile(normalized);
    if (profile != nullptr)
        m_profiles.insert(normalized, profile);

    return profile;
}

bool ProfileManager::removeDirectoryTree(const QString &directoryPath, QString *errorMessage) const
{
    QDir directory(directoryPath);
    if (!directory.exists())
        return true;

    if (!directory.removeRecursively()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to remove directory: %1").arg(directoryPath);
        }
        qCWarning(profileLog) << "Failed to remove directory" << directoryPath;
        return false;
    }

    return true;
}

bool ProfileManager::directoryContainsUserData(const QString &directoryPath) const
{
    QDir directory(directoryPath);
    if (!directory.exists())
        return false;

    const QFileInfoList entries = directory.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    return !entries.isEmpty();
}
