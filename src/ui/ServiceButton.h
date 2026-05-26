#pragma once

#include "services/ServiceState.h"

#include <QIcon>
#include <QWidget>

class ServiceButton : public QWidget
{
    Q_OBJECT

public:
    static constexpr int kIconLogicalSize = 40;

    explicit ServiceButton(const QString &serviceId,
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
    void applyPlaceholderIcon();

    QString m_serviceId;
    QString m_displayName;
    bool m_available = false;
    bool m_active = false;
    ServiceState m_state = ServiceState::Closed;
    qint64 m_iconCacheKey = 0;

    class QToolButton *m_serviceButton = nullptr;
    class QToolButton *m_stateButton = nullptr;
};
