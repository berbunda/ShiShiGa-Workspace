#pragma once

#include <QObject>

class QAction;
class MainWindow;
class QMenu;
class QSystemTrayIcon;
class SettingsManager;

class ApplicationTray : public QObject
{
    Q_OBJECT

public:
    ApplicationTray(MainWindow &mainWindow, SettingsManager &settings, QObject *parent = nullptr);

    bool isActive() const;
    void ensureVisible();
    void refreshToggleActionLabel();

public slots:
    void toggleMainWindowVisibility();
    void quitApplication();

private:
    void setup();

    MainWindow &m_mainWindow;
    SettingsManager &m_settings;

    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_contextMenu = nullptr;
    QAction *m_toggleAction = nullptr;
};
