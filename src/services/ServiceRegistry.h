#pragma once

#include "services/AiService.h"

#include <QList>
#include <QString>
#include <QUrl>

struct ServiceDefinition
{
    QString id;
    QString displayName;
    QUrl defaultUrl;
    bool available = false;
};

class ServiceRegistry
{
public:
    static QList<ServiceDefinition> allServices();
    static ServiceDefinition definitionFor(const QString &serviceId);
};
