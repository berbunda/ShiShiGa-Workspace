#include "WebEngineDiagnosticsInfo.h"

#include "WebEngineConsoleLog.h"
#include "core/ServiceManager.h"
#include "profile/ProfileManager.h"
#include "services/ServiceRegistry.h"

#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace {

QString na()
{
    return QStringLiteral("N/A");
}

} // namespace

WebEngineDiagnosticsSnapshot WebEngineDiagnosticsInfo::collect(const ServiceManager *serviceManager)
{
    WebEngineDiagnosticsSnapshot snapshot;
    if (serviceManager == nullptr || serviceManager->activeServiceId().isEmpty())
        return snapshot;

    const QString serviceId = serviceManager->activeServiceId();
    const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(serviceId);
    const QString profileId = catalog.profileId.isEmpty() ? catalog.id : catalog.profileId;

    snapshot.hasActiveService = true;
    snapshot.profileName = profileId;
    snapshot.documentLoadStatus = serviceManager->activeDocumentLoadStatusLabel();

    QWebEngineView *view = serviceManager->viewFor(serviceId);
    QWebEnginePage *page = serviceManager->pageFor(serviceId);
    if (view == nullptr || page == nullptr) {
        snapshot.currentUrl = catalog.defaultUrl.isValid() ? catalog.defaultUrl.toString() : na();
        snapshot.pageTitle = na();
        snapshot.faviconUrl = na();
        snapshot.userAgent = na();
        return snapshot;
    }

    snapshot.currentUrl = view->url().isValid() ? view->url().toString() : na();
    snapshot.pageTitle = view->title().isEmpty() ? na() : view->title();
    snapshot.faviconUrl = view->iconUrl().isValid() ? view->iconUrl().toString() : na();

    if (QWebEngineProfile *profile = page->profile(); profile != nullptr) {
        snapshot.userAgent = profile->httpUserAgent().isEmpty() ? na() : profile->httpUserAgent();
    } else {
        snapshot.userAgent = na();
    }

    return snapshot;
}

QString WebEngineDiagnosticsInfo::formatWebEngineSection(const WebEngineDiagnosticsSnapshot &snapshot)
{
    QString report;

    auto appendLine = [&report](const QString &key, const QString &value) {
        report += key;
        report += QStringLiteral(": ");
        report += value;
        report += QStringLiteral("\n");
    };

    report += QStringLiteral("WebEngine\n");
    report += QString(40, QLatin1Char('-'));
    report += QStringLiteral("\n");

    if (!snapshot.hasActiveService) {
        appendLine(QStringLiteral("Status"), QStringLiteral("No active service"));
        return report;
    }

    appendLine(QStringLiteral("Current URL"), snapshot.currentUrl);
    appendLine(QStringLiteral("Page Title"), snapshot.pageTitle);
    appendLine(QStringLiteral("Favicon URL"), snapshot.faviconUrl);
    appendLine(QStringLiteral("Profile Name"), snapshot.profileName);
    appendLine(QStringLiteral("User-Agent"), snapshot.userAgent);
    report += QStringLiteral("\n");

    report += QStringLiteral("Network Information\n");
    report += QString(40, QLatin1Char('-'));
    report += QStringLiteral("\n");
    appendLine(QStringLiteral("Current page URL"), snapshot.currentUrl);
    appendLine(QStringLiteral("Main document load status"), snapshot.documentLoadStatus);

    return report;
}

QString WebEngineDiagnosticsInfo::formatConsoleMessages(const ServiceManager *serviceManager)
{
    if (serviceManager == nullptr || serviceManager->activeServiceId().isEmpty())
        return QStringLiteral("No active service.\n");

    const WebEngineConsoleLog *consoleLog = serviceManager->webEngineConsoleLog();
    if (consoleLog == nullptr)
        return QStringLiteral("Console log unavailable.\n");

    const QList<WebEngineConsoleEntry> entries = consoleLog->messages(serviceManager->activeServiceId());
    if (entries.isEmpty())
        return QStringLiteral("(no console messages)\n");

    QString report;
    for (const WebEngineConsoleEntry &entry : entries) {
        report += QStringLiteral("[%1] [%2] %3\n")
                      .arg(entry.timestamp.toString(QStringLiteral("HH:mm:ss.zzz")),
                           consoleLevelLabel(entry.level),
                           entry.message);
        if (!entry.sourceId.isEmpty() || entry.lineNumber > 0) {
            report += QStringLiteral("    at %1:%2\n")
                          .arg(entry.sourceId.isEmpty() ? na() : entry.sourceId)
                          .arg(entry.lineNumber);
        }
    }

    return report;
}

QString WebEngineDiagnosticsInfo::consoleLevelLabel(const int level)
{
    switch (level) {
    case 0:
        return QStringLiteral("log");
    case 1:
        return QStringLiteral("warn");
    case 2:
        return QStringLiteral("error");
    default:
        return QStringLiteral("info");
    }
}
