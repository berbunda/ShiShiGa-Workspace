#include "SettingsManager.h"

#include <QMutexLocker>
#include <QSettings>

#include <algorithm>

namespace {

constexpr int kDefaultFontSize = SettingsManager::kDefaultFontSize;
constexpr int kDefaultSidebarWidth = 88;
constexpr int kDefaultAutoUnloadTimeoutMinutes = SettingsManager::kDefaultAutoUnloadTimeoutMinutes;
constexpr int kDefaultMainWindowWidth = 1400;
constexpr int kDefaultMainWindowHeight = 900;
constexpr int kUnsetWindowCoordinate = -1;

constexpr char kOrganizationName[] = "ShiShiga";
constexpr char kApplicationName[] = "ShiShiga Workspace";

constexpr char kGroupUi[] = "UI";
constexpr char kGroupBehavior[] = "Behavior";
constexpr char kGroupWindow[] = "Window";

constexpr char kKeyFontSize[] = "FontSize";
constexpr char kKeySidebarWidth[] = "SidebarWidth";
constexpr char kKeyAutoUnloadTimeoutMinutes[] = "AutoUnloadTimeoutMinutes";
constexpr char kKeyMainWindowWidth[] = "MainWindowWidth";
constexpr char kKeyMainWindowHeight[] = "MainWindowHeight";
constexpr char kKeyMainWindowX[] = "MainWindowX";
constexpr char kKeyMainWindowY[] = "MainWindowY";
constexpr char kKeyMainWindowMaximized[] = "MainWindowMaximized";
constexpr char kKeyRememberMainWindowGeometry[] = "RememberMainWindowGeometry";

QSettings makeSettings()
{
    return QSettings(QSettings::IniFormat,
                     QSettings::UserScope,
                     QString::fromLatin1(kOrganizationName),
                     QString::fromLatin1(kApplicationName));
}

} // namespace

SettingsManager &SettingsManager::instance()
{
    static SettingsManager manager;
    return manager;
}

SettingsManager::SettingsManager()
{
    applyDefaults();
}

void SettingsManager::load()
{
    QMutexLocker locker(&m_mutex);
    readFromSettings();
}

void SettingsManager::save()
{
    QMutexLocker locker(&m_mutex);
    writeToSettings();
}

void SettingsManager::resetToDefaults()
{
    QMutexLocker locker(&m_mutex);
    applyDefaults();
    writeToSettings();
}

QString SettingsManager::storageLocation()
{
    return makeSettings().fileName();
}

int SettingsManager::fontSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_fontSize;
}

void SettingsManager::setFontSize(int size)
{
    QMutexLocker locker(&m_mutex);
    m_fontSize = std::clamp(size, kMinFontSize, kMaxFontSize);
}

int SettingsManager::sidebarWidth() const
{
    QMutexLocker locker(&m_mutex);
    return m_sidebarWidth;
}

void SettingsManager::setSidebarWidth(int width)
{
    QMutexLocker locker(&m_mutex);
    m_sidebarWidth = width > 0 ? width : kDefaultSidebarWidth;
}

int SettingsManager::autoUnloadTimeoutMinutes() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoUnloadTimeoutMinutes;
}

int SettingsManager::autoUnloadTimeoutMs() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoUnloadTimeoutMinutes * 60 * 1000;
}

void SettingsManager::setAutoUnloadTimeoutMinutes(int minutes)
{
    QMutexLocker locker(&m_mutex);
    m_autoUnloadTimeoutMinutes = std::clamp(minutes,
                                            kMinAutoUnloadTimeoutMinutes,
                                            kMaxAutoUnloadTimeoutMinutes);
}

QSize SettingsManager::mainWindowSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_mainWindowSize;
}

void SettingsManager::setMainWindowSize(const QSize &size)
{
    QMutexLocker locker(&m_mutex);
    m_mainWindowSize = size.isValid() && size.width() > 0 && size.height() > 0
        ? size
        : QSize(kDefaultMainWindowWidth, kDefaultMainWindowHeight);
}

QPoint SettingsManager::mainWindowPosition() const
{
    QMutexLocker locker(&m_mutex);
    return m_mainWindowPosition;
}

void SettingsManager::setMainWindowPosition(const QPoint &position)
{
    QMutexLocker locker(&m_mutex);
    m_mainWindowPosition = position;
}

bool SettingsManager::mainWindowMaximized() const
{
    QMutexLocker locker(&m_mutex);
    return m_mainWindowMaximized;
}

void SettingsManager::setMainWindowMaximized(bool maximized)
{
    QMutexLocker locker(&m_mutex);
    m_mainWindowMaximized = maximized;
}

bool SettingsManager::rememberMainWindowGeometry() const
{
    QMutexLocker locker(&m_mutex);
    return m_rememberMainWindowGeometry;
}

void SettingsManager::setRememberMainWindowGeometry(bool remember)
{
    QMutexLocker locker(&m_mutex);
    m_rememberMainWindowGeometry = remember;
}

void SettingsManager::applyDefaults()
{
    m_fontSize = kDefaultFontSize;
    m_sidebarWidth = kDefaultSidebarWidth;
    m_autoUnloadTimeoutMinutes = kDefaultAutoUnloadTimeoutMinutes;
    m_mainWindowSize = QSize(kDefaultMainWindowWidth, kDefaultMainWindowHeight);
    m_mainWindowPosition = QPoint(kUnsetWindowCoordinate, kUnsetWindowCoordinate);
    m_mainWindowMaximized = false;
    m_rememberMainWindowGeometry = SettingsManager::kDefaultRememberMainWindowGeometry;
}

void SettingsManager::readFromSettings()
{
    QSettings settings = makeSettings();

    settings.beginGroup(QString::fromLatin1(kGroupUi));
    m_fontSize = settings.value(QString::fromLatin1(kKeyFontSize), kDefaultFontSize).toInt();
    m_sidebarWidth = settings.value(QString::fromLatin1(kKeySidebarWidth), kDefaultSidebarWidth).toInt();
    settings.endGroup();

    settings.beginGroup(QString::fromLatin1(kGroupBehavior));
    m_autoUnloadTimeoutMinutes = settings
        .value(QString::fromLatin1(kKeyAutoUnloadTimeoutMinutes), kDefaultAutoUnloadTimeoutMinutes)
        .toInt();
    settings.endGroup();

    settings.beginGroup(QString::fromLatin1(kGroupWindow));
    const int width = settings.value(QString::fromLatin1(kKeyMainWindowWidth), kDefaultMainWindowWidth).toInt();
    const int height = settings.value(QString::fromLatin1(kKeyMainWindowHeight), kDefaultMainWindowHeight).toInt();
    m_mainWindowSize = QSize(width, height);
    m_mainWindowPosition = QPoint(
        settings.value(QString::fromLatin1(kKeyMainWindowX), kUnsetWindowCoordinate).toInt(),
        settings.value(QString::fromLatin1(kKeyMainWindowY), kUnsetWindowCoordinate).toInt());
    m_mainWindowMaximized = settings
        .value(QString::fromLatin1(kKeyMainWindowMaximized), false)
        .toBool();
    m_rememberMainWindowGeometry = settings
        .value(QString::fromLatin1(kKeyRememberMainWindowGeometry),
               SettingsManager::kDefaultRememberMainWindowGeometry)
        .toBool();
    settings.endGroup();

    if (m_fontSize <= 0)
        m_fontSize = kDefaultFontSize;
    if (m_sidebarWidth <= 0)
        m_sidebarWidth = kDefaultSidebarWidth;
    if (m_autoUnloadTimeoutMinutes <= 0)
        m_autoUnloadTimeoutMinutes = kDefaultAutoUnloadTimeoutMinutes;
    if (!m_mainWindowSize.isValid() || m_mainWindowSize.width() <= 0 || m_mainWindowSize.height() <= 0)
        m_mainWindowSize = QSize(kDefaultMainWindowWidth, kDefaultMainWindowHeight);
}

void SettingsManager::writeToSettings()
{
    QSettings settings = makeSettings();

    settings.beginGroup(QString::fromLatin1(kGroupUi));
    settings.setValue(QString::fromLatin1(kKeyFontSize), m_fontSize);
    settings.setValue(QString::fromLatin1(kKeySidebarWidth), m_sidebarWidth);
    settings.endGroup();

    settings.beginGroup(QString::fromLatin1(kGroupBehavior));
    settings.setValue(QString::fromLatin1(kKeyAutoUnloadTimeoutMinutes), m_autoUnloadTimeoutMinutes);
    settings.endGroup();

    settings.beginGroup(QString::fromLatin1(kGroupWindow));
    settings.setValue(QString::fromLatin1(kKeyMainWindowWidth), m_mainWindowSize.width());
    settings.setValue(QString::fromLatin1(kKeyMainWindowHeight), m_mainWindowSize.height());
    settings.setValue(QString::fromLatin1(kKeyMainWindowX), m_mainWindowPosition.x());
    settings.setValue(QString::fromLatin1(kKeyMainWindowY), m_mainWindowPosition.y());
    settings.setValue(QString::fromLatin1(kKeyMainWindowMaximized), m_mainWindowMaximized);
    settings.setValue(QString::fromLatin1(kKeyRememberMainWindowGeometry), m_rememberMainWindowGeometry);
    settings.endGroup();

    settings.sync();
}
