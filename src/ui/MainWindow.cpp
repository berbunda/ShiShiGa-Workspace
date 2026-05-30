#include "MainWindow.h"

#include "DebugConsoleWindow.h"
#include "DevConsoleWindow.h"
#include "ServiceSidebar.h"
#include "SettingsWindow.h"
#include "core/ServiceManager.h"
#include "core/SettingsManager.h"
#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QPoint>
#include <QStackedWidget>
#include <QWidget>

namespace {

constexpr char kInitialPositionSetProperty[] = "initialPositionSet";

void positionConsoleWindow(QWidget *window, const QWidget *mainWindow, const QPoint &offset)
{
    if (window->property(kInitialPositionSetProperty).toBool())
        return;

    const QPoint anchor = mainWindow->frameGeometry().topRight();
    window->move(anchor + offset);
    window->setProperty(kInitialPositionSetProperty, true);
}

void showConsoleWindow(QWidget *window, QWidget *mainWindow, const QPoint &offset)
{
    positionConsoleWindow(window, mainWindow, offset);
    window->show();
    window->raise();
    window->activateWindow();
}

} // namespace

MainWindow::MainWindow(SettingsManager &settings, QWidget *parent)
    : QMainWindow(parent)
    , m_settings(settings)
{
    setWindowTitle(QStringLiteral("ShiShiga Workspace"));
    setupMenu();
    applyWindowSettings();
    applyApplicationFont();
    setupUi();
}

void MainWindow::applyWindowSettings()
{
    if (!m_settings.rememberMainWindowGeometry())
        return;

    if (m_settings.mainWindowMaximized()) {
        showMaximized();
        return;
    }

    resize(m_settings.mainWindowSize());

    const QPoint position = m_settings.mainWindowPosition();
    if (position.x() >= 0 && position.y() >= 0)
        move(position);
}

void MainWindow::applyApplicationFont()
{
    QFont font = QApplication::font();
    font.setPointSize(m_settings.fontSize());
    QApplication::setFont(font);
}

void MainWindow::persistWindowSettings()
{
    if (!m_settings.rememberMainWindowGeometry())
        return;

    if (isMaximized()) {
        m_settings.setMainWindowMaximized(true);
        return;
    }

    m_settings.setMainWindowMaximized(false);
    m_settings.setMainWindowSize(size());
    m_settings.setMainWindowPosition(pos());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    persistWindowSettings();
    m_settings.save();
    QMainWindow::closeEvent(event);
}

void MainWindow::setupMenu()
{
    auto *settingsMenu = menuBar()->addMenu(tr("Settings"));
    auto *preferencesAction = settingsMenu->addAction(tr("Preferences..."));
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::openSettings);

    auto *toolsMenu = menuBar()->addMenu(tr("Tools"));
    auto *debugConsoleAction = toolsMenu->addAction(tr("Debug Console"));
    auto *devConsoleAction = toolsMenu->addAction(tr("Dev Console"));
    connect(debugConsoleAction, &QAction::triggered, this, &MainWindow::openDebugConsole);
    connect(devConsoleAction, &QAction::triggered, this, &MainWindow::openDevConsole);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_stack = new QStackedWidget(central);
    m_serviceManager = new ServiceManager(m_stack, this);
    m_sidebar = new ServiceSidebar(m_serviceManager, m_settings, central);

    layout->addWidget(m_sidebar);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);
}

void MainWindow::openSettings()
{
    if (m_settingsWindow == nullptr) {
        m_settingsWindow = new SettingsWindow(m_settings, m_serviceManager, this);
        connect(m_settingsWindow, &SettingsWindow::settingsApplied,
                this, &MainWindow::applyLiveSettings);
        connect(m_settingsWindow, &QObject::destroyed, this, [this]() {
            m_settingsWindow = nullptr;
        });
    }

    m_settingsWindow->show();
    m_settingsWindow->raise();
    m_settingsWindow->activateWindow();
}

void MainWindow::openDebugConsole()
{
    if (m_debugConsoleWindow == nullptr) {
        m_debugConsoleWindow = new DebugConsoleWindow(m_serviceManager, this);
        connect(m_debugConsoleWindow, &QObject::destroyed, this, [this]() {
            m_debugConsoleWindow = nullptr;
        });
    }

    showConsoleWindow(m_debugConsoleWindow, this, QPoint(32, 64));
}

void MainWindow::openDevConsole()
{
    if (m_devConsoleWindow == nullptr) {
        m_devConsoleWindow = new DevConsoleWindow(m_serviceManager, this);
        connect(m_devConsoleWindow, &QObject::destroyed, this, [this]() {
            m_devConsoleWindow = nullptr;
        });
    }

    showConsoleWindow(m_devConsoleWindow, this, QPoint(24, 48));
}

void MainWindow::applyLiveSettings()
{
    applyApplicationFont();

    if (m_serviceManager != nullptr)
        m_serviceManager->refreshInactivityTimeouts();
}
