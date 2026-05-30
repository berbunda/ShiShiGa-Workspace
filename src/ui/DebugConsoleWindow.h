#pragma once

#include <QDialog>

class QPlainTextEdit;
class QPushButton;
class QShowEvent;
class ServiceManager;

class DebugConsoleWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DebugConsoleWindow(ServiceManager *serviceManager, QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void refresh();
    void copyDebugInfo();
    void openProfileFolder();
    void openCrashesFolder();
    void onActiveServiceChanged(const QString &serviceId);

private:
    void setupUi();
    void connectSignals();

    ServiceManager *m_serviceManager = nullptr;
    QPlainTextEdit *m_output = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QPushButton *m_copyButton = nullptr;
    QPushButton *m_openProfileButton = nullptr;
    QPushButton *m_openCrashesButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};
