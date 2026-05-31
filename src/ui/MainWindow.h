#pragma once

#include <QMainWindow>

class AboutWindow;
class QCloseEvent;
class DebugConsoleWindow;
class DevConsoleWindow;
class LicenseWindow;
class ServiceManager;
class ServiceSidebar;
class SettingsManager;
class SettingsWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SettingsManager &settings, QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openSettings();
    void openDebugConsole();
    void openDevConsole();
    void openAbout();
    void openLicense();
    void applyLiveSettings();

private:
    void applyWindowSettings();
    void persistWindowSettings();
    void setupUi();
    void setupMenu();
    void applyApplicationFont();

    SettingsManager &m_settings;

    class QStackedWidget *m_stack = nullptr;
    ServiceManager *m_serviceManager = nullptr;
    ServiceSidebar *m_sidebar = nullptr;
    SettingsWindow *m_settingsWindow = nullptr;
    DebugConsoleWindow *m_debugConsoleWindow = nullptr;
    DevConsoleWindow *m_devConsoleWindow = nullptr;
    AboutWindow *m_aboutWindow = nullptr;
    LicenseWindow *m_licenseWindow = nullptr;
};