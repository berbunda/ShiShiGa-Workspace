#pragma once

#include <QString>

struct WebEngineDiagnosticsSnapshot
{
    bool hasActiveService = false;
    QString currentUrl;
    QString pageTitle;
    QString faviconUrl;
    QString profileName;
    QString userAgent;
    QString documentLoadStatus;
};

class ServiceManager;

class WebEngineDiagnosticsInfo
{
public:
    static WebEngineDiagnosticsSnapshot collect(const ServiceManager *serviceManager);
    static QString formatWebEngineSection(const WebEngineDiagnosticsSnapshot &snapshot);
    static QString formatConsoleMessages(const ServiceManager *serviceManager);
    static QString consoleLevelLabel(int level);
};
