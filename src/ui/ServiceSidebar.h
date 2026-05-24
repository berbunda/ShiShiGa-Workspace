#pragma once

#include <QHash>
#include <QWidget>

class ServiceManager;
class ServiceSidebarButton;

class ServiceSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceSidebar(ServiceManager *serviceManager, QWidget *parent = nullptr);

private:
    void rebuildButtons();
    void syncButtonState(const QString &serviceId);

    ServiceManager *m_serviceManager = nullptr;
    QWidget *m_buttonContainer = nullptr;
    QHash<QString, ServiceSidebarButton *> m_buttons;
};
