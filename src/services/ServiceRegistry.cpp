#include "ServiceRegistry.h"

QList<ServiceDefinition> ServiceRegistry::allServices()
{
    return {
        {
            QString::fromLatin1(AiService::ChatGpt),
            QStringLiteral("ChatGPT"),
            AiService::urlFor(AiService::ChatGpt),
            QUrl(QStringLiteral("https://chatgpt.com/favicon.ico")),
            true,
        },
        {
            QString::fromLatin1(AiService::Claude),
            QStringLiteral("Claude"),
            AiService::urlFor(AiService::Claude),
            QUrl(QStringLiteral("https://claude.ai/favicon.ico")),
            false,
        },
        {
            QString::fromLatin1(AiService::Gemini),
            QStringLiteral("Gemini"),
            AiService::urlFor(AiService::Gemini),
            QUrl(QStringLiteral("https://gemini.google.com/favicon.ico")),
            false,
        },
    };
}

ServiceDefinition ServiceRegistry::definitionFor(const QString &serviceId)
{
    for (const ServiceDefinition &definition : allServices()) {
        if (definition.id == serviceId)
            return definition;
    }
    return {};
}
