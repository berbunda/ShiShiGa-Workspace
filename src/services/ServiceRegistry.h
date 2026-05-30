#pragma once

#include <QIcon>
#include <QList>
#include <QString>
#include <QUrl>

struct ServiceCatalogEntry
{
    QString id;
    QString displayName;
    QUrl defaultUrl;
    QString profileId;
    QString faviconResourcePath;
    bool launchable = false;
};

class ServiceRegistry
{
public:
    static QList<ServiceCatalogEntry> catalog();
    static QList<ServiceCatalogEntry> launchableServices();
    static ServiceCatalogEntry entryFor(const QString &serviceId);
    static bool isLaunchable(const QString &serviceId);
    static QIcon faviconFor(const ServiceCatalogEntry &entry, int logicalSize = 40);
};
