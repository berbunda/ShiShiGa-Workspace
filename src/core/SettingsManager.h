#pragma once

#include <QMutex>
#include <QPoint>
#include <QSize>
#include <QString>

class SettingsManager
{
public:
    static constexpr int kMinFontSize = 8;
    static constexpr int kMaxFontSize = 32;
    static constexpr int kMinAutoUnloadTimeoutMinutes = 1;
    static constexpr int kMaxAutoUnloadTimeoutMinutes = 480;

    static constexpr int kDefaultFontSize = 14;
    static constexpr int kDefaultAutoUnloadTimeoutMinutes = 30;
    static constexpr bool kDefaultRememberMainWindowGeometry = true;

    static SettingsManager &instance();

    void load();
    void save();
    void resetToDefaults();

    static QString storageLocation();

    int fontSize() const;
    void setFontSize(int size);

    int sidebarWidth() const;
    void setSidebarWidth(int width);

    int autoUnloadTimeoutMinutes() const;
    int autoUnloadTimeoutMs() const;
    void setAutoUnloadTimeoutMinutes(int minutes);

    QSize mainWindowSize() const;
    void setMainWindowSize(const QSize &size);

    QPoint mainWindowPosition() const;
    void setMainWindowPosition(const QPoint &position);

    bool mainWindowMaximized() const;
    void setMainWindowMaximized(bool maximized);

    bool rememberMainWindowGeometry() const;
    void setRememberMainWindowGeometry(bool remember);

private:
    SettingsManager();

    void applyDefaults();
    void readFromSettings();
    void writeToSettings();

    mutable QMutex m_mutex;

    int m_fontSize = 0;
    int m_sidebarWidth = 0;
    int m_autoUnloadTimeoutMinutes = 0;

    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    bool m_mainWindowMaximized = false;
    bool m_rememberMainWindowGeometry = true;
};
