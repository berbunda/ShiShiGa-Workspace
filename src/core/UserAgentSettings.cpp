#include "UserAgentSettings.h"

#include "SettingsManager.h"

#include <QWebEngineProfile>

#include <iterator>

namespace {

const UserAgentPreset kPresets[] = {
    {
        QStringLiteral("firefox"),
        QStringLiteral("Firefox"),
        QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:135.0) Gecko/20100101 Firefox/135.0"),
    },
    {
        QStringLiteral("chrome"),
        QStringLiteral("Chrome"),
        QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36"),
    },
    {
        QStringLiteral("edge"),
        QStringLiteral("Edge"),
        QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0"),
    },
};

} // namespace

QList<UserAgentPreset> UserAgentSettings::presets()
{
    QList<UserAgentPreset> result;
    result.reserve(std::size(kPresets));
    for (const UserAgentPreset &preset : kPresets)
        result.append(preset);
    return result;
}

const UserAgentPreset *UserAgentSettings::presetById(const QString &presetId)
{
    for (const UserAgentPreset &preset : kPresets) {
        if (preset.id == presetId)
            return &preset;
    }
    return nullptr;
}

UserAgentMode UserAgentSettings::modeFromString(const QString &value)
{
    if (value.compare(QStringLiteral("Preset"), Qt::CaseInsensitive) == 0)
        return UserAgentMode::Preset;
    if (value.compare(QStringLiteral("Custom"), Qt::CaseInsensitive) == 0)
        return UserAgentMode::Custom;
    return UserAgentMode::Default;
}

QString UserAgentSettings::modeToString(UserAgentMode mode)
{
    switch (mode) {
    case UserAgentMode::Preset:
        return QStringLiteral("Preset");
    case UserAgentMode::Custom:
        return QStringLiteral("Custom");
    case UserAgentMode::Default:
        break;
    }
    return QStringLiteral("Default");
}

QString UserAgentSettings::resolve(UserAgentMode mode,
                                   const QString &presetId,
                                   const QString &customUserAgent)
{
    switch (mode) {
    case UserAgentMode::Preset: {
        const UserAgentPreset *preset = presetById(presetId);
        if (preset == nullptr)
            preset = presetById(QString::fromLatin1(kDefaultPresetId));
        return preset != nullptr ? preset->userAgent : QString();
    }
    case UserAgentMode::Custom:
        return customUserAgent.trimmed();
    case UserAgentMode::Default:
        break;
    }
    return {};
}

QString UserAgentSettings::resolve(const SettingsManager &settings)
{
    return resolve(settings.userAgentMode(),
                   settings.userAgentPresetId(),
                   settings.customUserAgent());
}

QString UserAgentSettings::displayLabel(UserAgentMode mode,
                                        const QString &presetId,
                                        const QString &customUserAgent)
{
    switch (mode) {
    case UserAgentMode::Preset: {
        const UserAgentPreset *preset = presetById(presetId);
        if (preset == nullptr)
            preset = presetById(QString::fromLatin1(kDefaultPresetId));
        return preset != nullptr ? preset->displayName : QStringLiteral("Preset");
    }
    case UserAgentMode::Custom: {
        const QString trimmed = customUserAgent.trimmed();
        return trimmed.isEmpty() ? QStringLiteral("Custom (empty)") : trimmed;
    }
    case UserAgentMode::Default:
        break;
    }
    return QStringLiteral("Default (Qt WebEngine)");
}

void UserAgentSettings::applyToProfile(QWebEngineProfile *profile)
{
    if (profile == nullptr)
        return;

    profile->setHttpUserAgent(SettingsManager::instance().resolvedUserAgent());
}
