#pragma once

#include <QMainWindow>

class ServiceManager;
class ServiceSidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    void openDefaultService();

    class QStackedWidget *m_stack = nullptr;
    ServiceManager *m_serviceManager = nullptr;
    ServiceSidebar *m_sidebar = nullptr;
};
