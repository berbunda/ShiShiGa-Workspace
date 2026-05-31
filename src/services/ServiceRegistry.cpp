#include "ServiceRegistry.h"

#include "core/ServiceFaviconProvider.h"

#include <iterator>

namespace {

struct ServiceDefinition
{
    const char *id;
    const char *displayName;
    const char *startUrl;
    const char *profileId;
    const char *faviconUrl;
    bool enabled;
};

// Central catalog of AI services. Append a ServiceDefinition row to add a service.
// profileId nullptr uses entry.id (isolated WebEngine profile per service).
constexpr ServiceDefinition kServiceDefinitions[] = {
    {"chatgpt", "ChatGPT", "https://chatgpt.com", nullptr, nullptr, true},
    {"claude", "Claude", "https://claude.ai/", nullptr, nullptr, true},
    {"gemini", "Google Gemini", "https://gemini.google.com/", nullptr, nullptr, true},
    {"deepseek", "DeepSeek", "https://chat.deepseek.com/", nullptr, nullptr, true},

    {"aistudio", "Google AI Studio", "https://aistudio.google.com/", nullptr, nullptr, true},
    {"labs", "Google Labs", "https://labs.google/", nullptr, nullptr, true},
    {"notebooklm", "Google NotebookLM", "https://notebooklm.google/", nullptr, nullptr, true},
    {"grok", "Grok", "https://grok.com/", nullptr, nullptr, true},
    {"higgsfield", "Higgsfield", "https://higgsfield.ai/", nullptr, nullptr, true},
    {"euria", "Infomaniak Euria", "https://euria.infomaniak.com/", nullptr, nullptr, true},
    {"copilot", "Microsoft Copilot", "https://copilot.microsoft.com/", nullptr, nullptr, true},
    {"minimax", "MiniMax", "https://agent.minimax.io/", nullptr, nullptr, true},
    {"mistral", "Mistral", "https://chat.mistral.ai/", nullptr, nullptr, true},
    {"nanobanana", "Nano Banana 2", "https://nanobanana.io/", nullptr, nullptr, true},
    {"perplexity", "Perplexity", "https://www.perplexity.ai/", nullptr, nullptr, true},
    {"poe", "Poe", "https://poe.com/", nullptr, nullptr, true},
    {"qwen", "Qwen Studio", "https://chat.qwen.ai/", nullptr, nullptr, true},
    {"scira", "Scira AI", "https://scira.ai/", nullptr, nullptr, true},
    {"zai", "Z.ai", "https://z.ai/chat", nullptr, nullptr, true},
    {"characterai", "Character.AI", "https://character.ai/", nullptr, nullptr, true},
};

ServiceCatalogEntry makeEntry(const ServiceDefinition &definition)
{
    ServiceCatalogEntry entry;
    entry.id = QString::fromLatin1(definition.id);
    entry.displayName = QString::fromUtf8(definition.displayName);
    entry.startUrl = QUrl(QString::fromLatin1(definition.startUrl));
    entry.profileId = definition.profileId != nullptr && definition.profileId[0] != '\0'
        ? QString::fromLatin1(definition.profileId)
        : entry.id;
    if (definition.faviconUrl != nullptr && definition.faviconUrl[0] != '\0')
        entry.faviconUrl = QUrl(QString::fromLatin1(definition.faviconUrl));
    entry.enabled = definition.enabled;
    return entry;
}

QList<ServiceCatalogEntry> buildCatalog()
{
    QList<ServiceCatalogEntry> catalog;
    catalog.reserve(static_cast<int>(std::size(kServiceDefinitions)));
    for (const ServiceDefinition &definition : kServiceDefinitions)
        catalog.append(makeEntry(definition));
    return catalog;
}

} // namespace

QList<ServiceCatalogEntry> ServiceRegistry::catalog()
{
    return buildCatalog();
}

QList<ServiceCatalogEntry> ServiceRegistry::enabledServices()
{
    QList<ServiceCatalogEntry> enabled;
    for (const ServiceCatalogEntry &entry : buildCatalog()) {
        if (entry.enabled)
            enabled.append(entry);
    }
    return enabled;
}

ServiceCatalogEntry ServiceRegistry::entryFor(const QString &serviceId)
{
    for (const ServiceCatalogEntry &entry : buildCatalog()) {
        if (entry.id == serviceId)
            return entry;
    }
    return {};
}

bool ServiceRegistry::isEnabled(const QString &serviceId)
{
    const ServiceCatalogEntry entry = entryFor(serviceId);
    return !entry.id.isEmpty() && entry.enabled;
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
