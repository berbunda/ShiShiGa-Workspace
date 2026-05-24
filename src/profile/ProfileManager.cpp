#include "ProfileManager.h"

#include "ProfileSecurity.h"

#include <QDir>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QWebEngineProfile>

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

QString ProfileManager::profilesRoot() const
{
    const QString localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    return QDir(localAppData).filePath(QStringLiteral("ShiShiga-Workspace/profiles"));
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
