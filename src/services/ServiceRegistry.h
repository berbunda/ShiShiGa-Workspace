#pragma once

#include <QIcon>
#include <QList>
#include <QString>
#include <QUrl>

struct ServiceCatalogEntry
{
    QString id;
    QString displayName;
    QUrl startUrl;
    QString profileId;
    QUrl faviconUrl;
    QString faviconResourcePath;
    bool enabled = false;
};

class ServiceRegistry
{
public:
    static QList<ServiceCatalogEntry> catalog();
    static QList<ServiceCatalogEntry> enabledServices();
    static ServiceCatalogEntry entryFor(const QString &serviceId);
    static bool isEnabled(const QString &serviceId);
    static QIcon faviconFor(const ServiceCatalogEntry &entry, int logicalSize = 40);
};
