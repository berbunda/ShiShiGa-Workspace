#pragma once

#include <QHash>
#include <QWidget>

class ServiceManager;
class ServiceButton;
class SettingsManager;

class ServiceSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceSidebar(ServiceManager *serviceManager,
                            SettingsManager &settings,
                            QWidget *parent = nullptr);

private:
    void rebuildButtons();
    void syncButtonState(const QString &serviceId);
    void syncButtonIcon(const QString &serviceId);

    SettingsManager &m_settings;
    ServiceManager *m_serviceManager = nullptr;
    QWidget *m_buttonContainer = nullptr;
    QHash<QString, ServiceButton *> m_buttons;
};
