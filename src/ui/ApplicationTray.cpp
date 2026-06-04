#include "ApplicationTray.h"

#include "MainWindow.h"
#include "core/AppIcons.h"
#include "core/SettingsManager.h"
#include "logging/CrashLogger.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

namespace {

constexpr char kTrayCategory[] = "SystemTray";

} // namespace

ApplicationTray::ApplicationTray(MainWindow &mainWindow,
                                 SettingsManager &settings,
                                 QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_settings(settings)
{
    setup();
}

bool ApplicationTray::isActive() const
{
    return m_trayIcon != nullptr;
}

void ApplicationTray::ensureVisible()
{
    if (m_trayIcon == nullptr)
        return;

    if (!m_trayIcon->icon().isNull())
        m_trayIcon->setIcon(AppIcons::trayIcon());

    m_trayIcon->setVisible(true);
    m_trayIcon->show();
}

void ApplicationTray::setup()
{
    const bool reportedAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    if (!reportedAvailable) {
        CrashLogger::instance().logOperationalError(
            QString::fromLatin1(kTrayCategory),
            QStringLiteral("System tray reported unavailable; attempting tray icon anyway"));
    }

    const QIcon icon = AppIcons::trayIcon();
    if (icon.isNull()) {
        CrashLogger::instance().logOperationalError(
            QString::fromLatin1(kTrayCategory),
            QStringLiteral("Tray icon resources are missing (expected %1 and %2); continuing without tray icon")
                .arg(AppIcons::trayIcon48ResourcePath(), AppIcons::trayIcon16ResourcePath()));
        return;
    }

    m_trayIcon = new QSystemTrayIcon(icon, this);
    m_trayIcon->setToolTip(QStringLiteral("ShiShiga Workspace"));

    m_contextMenu = new QMenu;
    m_toggleAction = m_contextMenu->addAction(QString());
    connect(m_toggleAction, &QAction::triggered, this, &ApplicationTray::toggleMainWindowVisibility);

    auto *exitAction = m_contextMenu->addAction(tr("Exit"));
    connect(exitAction, &QAction::triggered, this, &ApplicationTray::quitApplication);

    m_trayIcon->setContextMenu(m_contextMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            toggleMainWindowVisibility();
    });

    refreshToggleActionLabel();

    // Show after the event loop starts so Windows registers the notification icon reliably.
    QTimer::singleShot(0, this, [this]() { ensureVisible(); });
}

void ApplicationTray::refreshToggleActionLabel()
{
    if (m_toggleAction == nullptr)
        return;

    const bool visible = m_mainWindow.isVisible() && !m_mainWindow.isMinimized();
    m_toggleAction->setText(visible
        ? tr("Hide ShiShiga Workspace")
        : tr("Show ShiShiga Workspace"));
}

void ApplicationTray::toggleMainWindowVisibility()
{
    if (m_mainWindow.isVisible() && !m_mainWindow.isMinimized()) {
        m_mainWindow.hide();
        refreshToggleActionLabel();
        return;
    }

    m_mainWindow.show();
    m_mainWindow.raise();
    m_mainWindow.activateWindow();
    refreshToggleActionLabel();
}

void ApplicationTray::quitApplication()
{
    if (m_trayIcon != nullptr)
        m_trayIcon->hide();

    m_mainWindow.requestApplicationExit();
}
