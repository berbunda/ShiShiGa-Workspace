#include "ServiceRegistry.h"

QList<ServiceDefinition> ServiceRegistry::allServices()
{
    return {
        {
            QString::fromLatin1(AiService::ChatGpt),
            QStringLiteral("ChatGPT"),
            AiService::urlFor(AiService::ChatGpt),
            true,
        },
        {
            QString::fromLatin1(AiService::Claude),
            QStringLiteral("Claude"),
            AiService::urlFor(AiService::Claude),
            false,
        },
        {
            QString::fromLatin1(AiService::Gemini),
            QStringLiteral("Gemini"),
            AiService::urlFor(AiService::Gemini),
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
