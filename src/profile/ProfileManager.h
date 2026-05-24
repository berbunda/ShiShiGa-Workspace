#pragma once

#include <QHash>
#include <QObject>
#include <QString>

class QWebEngineProfile;

class ProfileManager : public QObject
{
    Q_OBJECT

public:
    static ProfileManager &instance();

    QWebEngineProfile *profileFor(const QString &serviceName);
    QString profilesRoot() const;

private:
    explicit ProfileManager(QObject *parent = nullptr);

    QString normalizeServiceName(const QString &serviceName) const;
    bool ensureProfileDirectories(const QString &serviceName,
                                  QString *storagePath,
                                  QString *cachePath) const;
    QWebEngineProfile *createProfile(const QString &serviceName);

    QHash<QString, QWebEngineProfile *> m_profiles;
};
