#pragma once

#include "services/ServiceState.h"

#include <QIcon>
#include <QWidget>

class ServiceSidebarButton : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceSidebarButton(const QString &serviceId,
                                  const QString &displayName,
                                  bool available,
                                  QWidget *parent = nullptr);

    QString serviceId() const;
    void setActive(bool active);
    void setServiceState(ServiceState state);
    void setServiceIcon(const QIcon &icon);

signals:
    void activateRequested(const QString &serviceId);
    void unloadRequested(const QString &serviceId);
    void closeRequested(const QString &serviceId);

private:
    void updateStateButton();
    void updateStyles();

    QString m_serviceId;
    QString m_displayName;
    bool m_available = false;
    bool m_active = false;
    ServiceState m_state = ServiceState::Closed;

    class QToolButton *m_serviceButton = nullptr;
    class QToolButton *m_stateButton = nullptr;
};
