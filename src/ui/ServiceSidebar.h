#pragma once

#include <QHash>
#include <QWidget>

class QScrollArea;
class QVBoxLayout;
class SidebarAddButton;
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

private slots:
    void showAddServiceMenu();
    void addServiceButton(const QString &serviceId);
    void removeServiceButton(const QString &serviceId);
    void syncButtonState(const QString &serviceId);
    void syncButtonIcon(const QString &serviceId);
    void syncActiveButton(const QString &serviceId);

private:
    void setupLayout();
    void setupAddButton();
    int nextServiceInsertIndex() const;

    SettingsManager &m_settings;
    ServiceManager *m_serviceManager = nullptr;

    QWidget *m_addHeader = nullptr;
    SidebarAddButton *m_addButton = nullptr;

    QScrollArea *m_servicesScrollArea = nullptr;
    QWidget *m_servicesContainer = nullptr;
    QVBoxLayout *m_servicesLayout = nullptr;

    QHash<QString, ServiceButton *> m_buttons;
};
