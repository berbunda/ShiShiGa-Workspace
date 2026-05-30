#pragma once

#include <QDialog>

class QPlainTextEdit;
class QPushButton;
class QShowEvent;
class ServiceManager;

class DevConsoleWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DevConsoleWindow(ServiceManager *serviceManager, QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void refresh();
    void clearConsole();
    void copyConsole();
    void reloadPage();
    void onActiveServiceChanged(const QString &serviceId);
    void onConsoleMessageLogged(const QString &serviceId);

private:
    void setupUi();
    void connectSignals();
    void updateAvailability();
    void refreshWebEngineSection();
    void refreshConsoleSection();

    ServiceManager *m_serviceManager = nullptr;
    QPlainTextEdit *m_webEngineOutput = nullptr;
    QPlainTextEdit *m_consoleOutput = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_copyButton = nullptr;
    QPushButton *m_reloadButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};
