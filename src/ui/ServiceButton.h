#pragma once

#include "services/ServiceState.h"

#include <QIcon>
#include <QWidget>

class ServiceButton : public QWidget
{
    Q_OBJECT

public:
    static constexpr int kIconLogicalSize = 40;
    static constexpr int kIconPadding = 8;
    static constexpr int kHorizontalMargin = 8;
    static constexpr int kVerticalMargin = 4;
    static constexpr int kContentSpacing = 6;
    static constexpr int kStateButtonSize = 24;

    static QSize iconTouchTargetSize();
    static QSize rowSizeHint();

    explicit ServiceButton(const QString &serviceId,
                           const QString &displayName,
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
    bool m_active = false;
    ServiceState m_state = ServiceState::Unloaded;
    qint64 m_iconCacheKey = 0;

    class QToolButton *m_serviceButton = nullptr;
    class QToolButton *m_stateButton = nullptr;
};
