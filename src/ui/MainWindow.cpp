#include "MainWindow.h"

#include "ServiceSidebar.h"
#include "SettingsWindow.h"
#include "core/ServiceManager.h"
#include "core/SettingsManager.h"
#include "services/AiService.h"

#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStackedWidget>
#include <QWidget>

MainWindow::MainWindow(SettingsManager &settings, QWidget *parent)
    : QMainWindow(parent)
    , m_settings(settings)
{
    setWindowTitle(QStringLiteral("ShiShiga Workspace"));
    setupMenu();
    applyWindowSettings();
    applyApplicationFont();
    setupUi();
    openDefaultService();
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
        m_settingsWindow = new SettingsWindow(m_settings, this);
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

void MainWindow::applyLiveSettings()
{
    applyApplicationFont();

    if (m_serviceManager != nullptr)
        m_serviceManager->refreshInactivityTimeouts();
}

void MainWindow::openDefaultService()
{
    if (m_settings.restorePreviousSession()) {
        // Session restore will activate the last opened services here.
    }

    m_serviceManager->activateService(QString::fromLatin1(AiService::ChatGpt));
}
