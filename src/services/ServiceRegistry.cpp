#include "ServiceRegistry.h"

#include "core/ServiceFaviconProvider.h"
#include "services/AiService.h"

namespace {

ServiceCatalogEntry makeEntry(const char *id,
                             const QString &displayName,
                             const QUrl &url,
                             bool launchable)
{
    ServiceCatalogEntry entry;
    entry.id = QString::fromLatin1(id);
    entry.displayName = displayName;
    entry.defaultUrl = url;
    entry.profileId = entry.id;
    entry.launchable = launchable;
    return entry;
}

} // namespace

QList<ServiceCatalogEntry> ServiceRegistry::catalog()
{
    return {
        makeEntry(AiService::ChatGpt,
                  QStringLiteral("ChatGPT"),
                  AiService::urlFor(AiService::ChatGpt),
                  true),
        makeEntry(AiService::Claude,
                  QStringLiteral("Claude"),
                  AiService::urlFor(AiService::Claude),
                  false),
        makeEntry(AiService::Gemini,
                  QStringLiteral("Gemini"),
                  AiService::urlFor(AiService::Gemini),
                  false),
        makeEntry(AiService::DeepSeek,
                  QStringLiteral("DeepSeek"),
                  AiService::urlFor(AiService::DeepSeek),
                  false),
    };
}

QList<ServiceCatalogEntry> ServiceRegistry::launchableServices()
{
    QList<ServiceCatalogEntry> launchable;
    for (const ServiceCatalogEntry &entry : catalog()) {
        if (entry.launchable)
            launchable.append(entry);
    }
    return launchable;
}

ServiceCatalogEntry ServiceRegistry::entryFor(const QString &serviceId)
{
    for (const ServiceCatalogEntry &entry : catalog()) {
        if (entry.id == serviceId)
            return entry;
    }
    return {};
}

bool ServiceRegistry::isLaunchable(const QString &serviceId)
{
    const ServiceCatalogEntry entry = entryFor(serviceId);
    return !entry.id.isEmpty() && entry.launchable;
}

QIcon ServiceRegistry::faviconFor(const ServiceCatalogEntry &entry, int logicalSize)
{
    if (!entry.faviconResourcePath.isEmpty()) {
        const QIcon resourceIcon(entry.faviconResourcePath);
        if (!resourceIcon.isNull())
            return ServiceFaviconProvider::normalizedIcon(resourceIcon, logicalSize);
    }

    return ServiceFaviconProvider::placeholderIcon(entry.displayName, logicalSize);
}
