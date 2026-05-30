#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class QWebEngineProfile;

enum class ProfileDataState {
    Missing,
    Empty,
    Present,
};

struct ManagedProfileInfo
{
    QString serviceId;
    QString serviceDisplayName;
    QString profileId;
    QString profileRootPath;
    QString storagePath;
    QString cachePath;
};

class ProfileManager : public QObject
{
    Q_OBJECT

public:
    static ProfileManager &instance();

    QList<ManagedProfileInfo> registeredProfiles() const;

    QWebEngineProfile *profileFor(const QString &serviceName);
    QString profilesRoot() const;

    QString profileRootPath(const QString &profileId) const;
    ProfileDataState dataState(const QString &profileId) const;
    bool hasActiveProfile(const QString &profileId) const;

    qint64 directorySizeOnDisk(const QString &directoryPath) const;

    void releaseProfile(const QString &profileId);
    bool clearProfileData(const QString &profileId, QString *errorMessage = nullptr);

signals:
    void profileDataCleared(const QString &profileId);

private:
    explicit ProfileManager(QObject *parent = nullptr);

    QString normalizeServiceName(const QString &serviceName) const;
    bool ensureProfileDirectories(const QString &serviceName,
                                  QString *storagePath,
                                  QString *cachePath) const;
    QWebEngineProfile *createProfile(const QString &serviceName);
    bool removeDirectoryTree(const QString &directoryPath, QString *errorMessage) const;
    bool directoryContainsUserData(const QString &directoryPath) const;

    QHash<QString, QWebEngineProfile *> m_profiles;
};
