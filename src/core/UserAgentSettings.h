#pragma once

#include <QList>
#include <QString>

class QWebEngineProfile;
class SettingsManager;

enum class UserAgentMode {
    Default = 0,
    Preset = 1,
    Custom = 2,
};

struct UserAgentPreset
{
    QString id;
    QString displayName;
    QString userAgent;
};

class UserAgentSettings
{
public:
    static constexpr char kDefaultPresetId[] = "firefox";

    static QList<UserAgentPreset> presets();
    static const UserAgentPreset *presetById(const QString &presetId);
    static UserAgentMode modeFromString(const QString &value);
    static QString modeToString(UserAgentMode mode);

    static QString resolve(UserAgentMode mode,
                           const QString &presetId,
                           const QString &customUserAgent);
    static QString resolve(const SettingsManager &settings);
    static QString displayLabel(UserAgentMode mode,
                                const QString &presetId,
                                const QString &customUserAgent);

    static void applyToProfile(QWebEngineProfile *profile);
};
